#include "pch_OP_SyncCameras.h"
#include "Synchroniser.h"
#include "CameraThread.h"

namespace TD_MoCap {
	//----------
	Synchroniser::Synchroniser()
	{
		this->requestUpdate();
	}

	//----------
	Synchroniser::~Synchroniser()
	{
		std::cout << "Synchroniser closing" << std::endl;
		this->workerThread.join();
	}

	//----------
	void 
		Synchroniser::checkConnections(const std::vector<Links::Output::ID>& newOutputIDs)
	{
		this->workerThread.performBlocking([&] {
			// gather current IDs
			std::vector<Links::Output::ID> currentOutputIDs;
			for (const auto& camera : this->syncMembers) {
				currentOutputIDs.push_back(camera.first);
			}
			// see if changed
			if (newOutputIDs != currentOutputIDs) {
				// rebuild syncMembers
				this->syncMembers.clear();
				for (const auto id : newOutputIDs) {
					auto camera = std::make_unique<SyncMember>();
					camera->input.subscribe(id);
					this->syncMembers.emplace(id, std::move(camera));
				}
				this->needsResync = true;
			}
		});
	}

	//----------
	void
		Synchroniser::requestResync()
	{
		this->needsResync = true;
	}

	//----------
	Utils::WorkerThread &
		Synchroniser::getThread()
	{
		return this->workerThread;
	}

	//----------
	void
		Synchroniser::requestUpdate()
	{
		this->workerThread.perform([this] {
			try {
				this->receiveAllFrames();

				if (this->needsResync) {
					this->resync();
				}

				// request next update
				if (!this->workerThread.isJoining()) {
					this->requestUpdate();
				}
			}
			catch (...) {
				// request next update
				if (!this->workerThread.isJoining()) {
					this->requestUpdate();
				}

				throw;
			}
		});
	}

	//----------
	void
		Synchroniser::receiveAllFrames()
	{
		// Move frames from inputs into indexedFrames
		// Note : This does not involve talking to the camera/cameraThread

		for (const auto& it : this->syncMembers) {
			auto& syncMember = it.second;
			auto frame = std::dynamic_pointer_cast<XimeaCameraFrame>(syncMember->input.receiveNextFrame(false));
			if (frame) {
				syncMember->indexedFrames.emplace(frame->metaData.frameIndex, frame);
			}
		}
	}

	//----------
	void
		Synchroniser::resync()
	{
		std::cout << "resync" << std::endl;

		// check that we have at least 2 syncMembers
		if (this->syncMembers.size() < 2) {
			throw(Exception("Insufficient cameras to perform a sync"));
		}

		// check that we have a cameraThread for all syncMembers
		std::map<Links::Output::ID, std::shared_ptr<CameraThread>> cameraThreads;
		for (const auto& it: this->syncMembers) {
			auto latestFrame = std::dynamic_pointer_cast<XimeaCameraFrame>(it.second->input.receiveLatestFrame(true));
			if (!latestFrame) {
				throw(Exception("Camera input does not yet have a frame"));
			}

			auto cameraThread = latestFrame->cameraThread.lock();
			if (!cameraThread) {
				throw(Exception("Cameras is not initialised"));
			}

			cameraThreads.emplace(it.first, cameraThread);
		}

		// acquire perform locks on all cameraThreads
		std::map<Links::Output::ID, std::shared_ptr<Utils::WorkerThread::PerformLock>> performLocks;
		{
			for (const auto& it : cameraThreads) {
				performLocks.emplace(it.first, it.second->getThread().acquirePerformLock());
			}
		}

		// Note
		// We perform all xiAPI actions from this thread
		// According to the Ximea documentation, this is permitted
		// Meanwhile, we pause all camera threads using performLocks

		// gather the cameras for convenience
		std::map<Links::Output::ID, xiAPIplus_Camera*> cameras;
		for (const auto& cameraThread : cameraThreads) {
			cameras.emplace(cameraThread.first, &cameraThread.second->getCamera());
		}

		// check trigger states on all cameras
		// Note : this often seems to fail when it shouldn't (i.e. camera driver reports incorrect values)
		if(this->checkCameraTriggers) {
			auto cameraIt = cameras.begin();

			// check first camera
			{
				auto camera = cameraIt->second;
				if (camera->GetGPOMode() != XI_GPO_MODE::XI_GPO_EXPOSURE_ACTIVE) {
					throw(Exception("Leader camera output trigger not active"));
				}
			}
			
			// check other cameras
			for (cameraIt++; cameraIt != cameras.end(); cameraIt++) {
				auto camera = cameraIt->second;
				auto gpiMode = camera->GetGPIMode();
				auto triggerSource = camera->GetTriggerSource();
				if (gpiMode != XI_GPI_MODE::XI_GPI_TRIGGER
					|| triggerSource == XI_TRG_SOURCE::XI_TRG_OFF
					|| triggerSource == XI_TRG_SOURCE::XI_TRG_SOFTWARE
					) {
					throw(Exception("Follower camera output trigger not active"));
				}
			}
		}

		// disable acquisition on all cameras
		for (const auto & camera : cameras) {
			camera.second->StopAcquisition();
		}

		// enable acquisition on all follower cameras (we presume their buffers are empty)
		{
			auto cameraIt = cameras.begin();
			for (cameraIt++; cameraIt != cameras.end(); cameraIt++) {
				auto camera = cameraIt->second;
				camera->StartAcquisition();
			}
		}

		// enable acquisiton on the first camera
		{
			cameras.begin()->second->StartAcquisition();
		}

		// capture a frame and log the timestamp data for each camera
		for (const auto& cameraIt : cameras) {
			auto camera = cameraIt.second;
			auto & syncMember = this->syncMembers[cameraIt.first];

			xiAPIplus_Image image;
			camera->GetNextImage(&image);
			auto frameData = image.GetXI_IMG();

			syncMember->frameNumberStart = frameData->acq_nframe;
			syncMember->timestampStart = std::chrono::seconds(frameData->tsSec) + std::chrono::microseconds(frameData->tsUSec);
		}

		this->needsResync = false;
	}
}
