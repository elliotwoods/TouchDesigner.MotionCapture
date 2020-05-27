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
		this->joining = true;
	}

	//----------
	void 
		Synchroniser::checkConnections(const std::vector<Links::Output::ID>& newOutputIDs)
	{
		this->workerThread.performBlocking([&] {
			// gather current IDss
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
	void
		Synchroniser::requestUpdate()
	{
		this->workerThread.perform([this] {
			this->receiveAllFrames();

			if (this->needsResync) {
				this->resync();
			}

			// request next update
			if (!this->joining) {
				this->requestUpdate();
			}
		});
	}

	//----------
	void
		Synchroniser::receiveAllFrames()
	{
		for (const auto& it : this->syncMembers) {
			auto& camera = it.second;
			auto frame = std::dynamic_pointer_cast<XimeaCameraFrame>(camera->input.receiveNextFrame(false));
			if (frame) {
				camera->indexedFrames.emplace(frame->metaData.frameIndex, frame);
				camera->cameraThread = frame->cameraThread;
			}
		}
	}

	//----------
	void
		Synchroniser::resync()
	{
		// check that we have at least 2 syncMembers
		if (this->syncMembers.size() < 2) {
			throw(Exception("Insufficient cameras to perform a sync"));
		}

		// check that we have a cameraThread for all syncMembers
		std::map<Links::Output::ID, std::shared_ptr<CameraThread>> cameraThreads;
		for (const auto& it: this->syncMembers) {
			auto cameraThread = it.second->cameraThread.lock();
			if (!cameraThread) {
				throw(Exception("One or more cameras are not correctly initialised"));
			}
			else {
				cameraThreads.emplace(it.first, cameraThread);
			}
		}

		// acquire perform locks on all cameraThreads
		std::map<Links::Output::ID, std::shared_ptr<Utils::WorkerThread::PerformLock>> performLocks;
		{
			for (const auto& it : cameraThreads) {
				performLocks.emplace(it.first, it.second->getThread().acquirePerformLock());
			}
		}

		// WARNING!
		// We perform all xiAPI actions from this thread
		// According to the Ximea documentation, this is permitted
		// Meanwhile, we pause all camera threads using performLocks

		// gather the cameras for convenience
		std::map<Links::Output::ID, xiAPIplus_Camera*> cameras;
		for (const auto& cameraThread : cameraThreads) {
			cameras.emplace(cameraThread.first, &cameraThread.second->getCamera());
		}

		// check trigger states on all cameras
		{
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
				if (camera->GetGPIMode() != XI_GPI_MODE::XI_GPI_TRIGGER
					|| camera->GetTriggerSource() == XI_TRG_SOURCE::XI_TRG_OFF
					|| camera->GetTriggerSource() == XI_TRG_SOURCE::XI_TRG_SOFTWARE) {
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
	}
}
