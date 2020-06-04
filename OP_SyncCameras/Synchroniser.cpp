#include "pch_OP_SyncCameras.h"
#include "Synchroniser.h"
#include "CameraThread.h"
#include "SynchronisedFrame.h"

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
		std::lock_guard<std::mutex> lockGuard(this->lockSyncMembers);
		for (const auto& syncMember : this->syncMembers) {
			syncMember.second->input.close();
		}

		// Manually join the thread
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
				this->workerThread.wakeOnPerformBlocking.clear();
				for (const auto id : newOutputIDs) {
					auto camera = std::make_unique<SyncMember>();
					camera->input.subscribe(id);
					this->workerThread.wakeOnPerformBlocking.insert(&camera->input.getChannel());
					this->syncMembers.emplace(id, std::move(camera));
				}
				this->leaderID = newOutputIDs.front();
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

	Utils::ParameterList&
		Synchroniser::getParameters()
	{
		return this->parameters.list;
	}

	//----------
	void
		Synchroniser::requestUpdate()
	{
		this->workerThread.perform([this] {
			try {
				if (!this->receiveAllFrames()) {

				}

				if (this->needsResync) {
					this->resync();
				}

				this->sendSynchronisedFrames();

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
	bool
		Synchroniser::receiveAllFrames()
	{
		// Move frames from inputs into indexedFrames
		// Note : This does not involve talking to the camera/cameraThread
		std::lock_guard<std::mutex> lockSyncMembers(this->lockSyncMembers);
		if (this->syncMembers.size() < 2) {
			return false;
		}

		auto timeout = std::chrono::milliseconds(10);
		for (const auto& it : this->syncMembers) {
			auto& syncMember = it.second;

			// always receive one frame
			auto frame = std::dynamic_pointer_cast<XimeaCameraFrame>(syncMember->input.receiveNextFrameWait(timeout));

			while (frame) {
				syncMember->indexedFrames.emplace(frame->metaData.frameIndex, frame);

				// receive any other frames that are waiting also
				frame = std::dynamic_pointer_cast<XimeaCameraFrame>(syncMember->input.receiveNextFrameWait(timeout));
			}

			// cap history size
			auto maxHistorySize = this->parameters.maxHistory.getValue();
			while (syncMember->indexedFrames.size() > maxHistorySize) {
				syncMember->indexedFrames.erase(syncMember->indexedFrames.begin());
			}
		}

		return true;
	}

	//----------
	void
		Synchroniser::resync()
	{
		std::lock_guard<std::mutex> lockSyncMembers(this->lockSyncMembers);
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
			// check first camera
			{
				auto camera = cameras[this->leaderID];
				if (camera->GetGPOMode() != XI_GPO_MODE::XI_GPO_EXPOSURE_ACTIVE) {
					throw(Exception("Leader camera output trigger not active"));
				}
			}
			
			// check other cameras
			for (const auto & cameraIt : cameras) {
				if (cameraIt.first == this->leaderID) {
					continue;
				}

				auto camera = cameraIt.second;
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
			for (const auto & cameraIt : cameras) {
				if (cameraIt.first == this->leaderID) {
					continue;
				}

				auto camera = cameraIt.second;
				camera->StartAcquisition();
			}
		}

		// enable acquisiton on the first camera
		{
			cameras[this->leaderID]->StartAcquisition();
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

	//----------
	void
		Synchroniser::sendSynchronisedFrames()
	{
		std::lock_guard<std::mutex> lockSyncMembers(this->lockSyncMembers);

		if (this->syncMembers.size() < 2) {
			// not enough cameras to perform a sync
			return;
		}

		auto& leader = this->syncMembers[this->leaderID];
		if (leader->indexedFrames.empty()) {
			// no frames in Leader camera
			return;
		}

		// check followers
		{
			for (const auto & it : this->syncMembers) {
				if (it.first == this->leaderID) {
					continue;
				}

				if (it.second->indexedFrames.empty()) {
					// one of the followers has no frames
					return;
				}
			}
		}

		const auto& strategy = this->parameters.strategy.getValue();
		if (strategy == "Frameindex") {
			// remove all frames which are too old
			// note : this is somewhat optional since we have the max history limit also
			auto oldestFrameIndex = 0;
			{

				// find the common oldestFrameIndex
				for (const auto& syncMemberIt : this->syncMembers) {
					auto& frames = syncMemberIt.second->indexedFrames;
					if (frames.empty()) {
						continue;
					}
					auto oldestFrameIndexForThisMember = frames.begin()->first - syncMemberIt.second->frameNumberStart;
					if (oldestFrameIndexForThisMember > oldestFrameIndex) {
						oldestFrameIndex = oldestFrameIndexForThisMember;
					}
				}

				// delete all the frames which will never be matched
				for (const auto& syncMemberIt : this->syncMembers) {
					auto& frames = syncMemberIt.second->indexedFrames;
					for (auto it = frames.begin(); it != frames.end(); ) {
						auto correctedFrameIndex = it->first - syncMemberIt.second->frameNumberStart;
						if (correctedFrameIndex < oldestFrameIndex) {
							it = frames.erase(it);
						}
						else {
							it++;
						}
					}
				}
			}

			// search for synched frames
			for (auto itLeaderFrame = leader->indexedFrames.begin(); itLeaderFrame != leader->indexedFrames.end(); ) {
				auto startCompute = std::chrono::high_resolution_clock::now();

				auto frameIndexGlobal = itLeaderFrame->first - leader->frameNumberStart;
				bool matchFound = true;
				
				for(const auto & itFollower : this->syncMembers) {
					if (itFollower.first == this->leaderID) {
						continue;
					}

					auto& follower = itFollower.second;
					if (follower->indexedFrames.find(frameIndexGlobal + follower->frameNumberStart) == follower->indexedFrames.end()) {
						matchFound = false;
						break;
					}
				}

				if (matchFound) {
					std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>> cameraFrames;

					// put leader frame
					cameraFrames.emplace(this->leaderID, itLeaderFrame->second);

					// put other frames
					for (const auto& itFollower : this->syncMembers) {
						if (itFollower.first == this->leaderID) {
							continue;
						}
						auto& follower = itFollower.second;

						auto followerFrameIndex = itLeaderFrame->first - leader->frameNumberStart + follower->frameNumberStart;
						cameraFrames.emplace(itFollower.first, follower->indexedFrames[followerFrameIndex]);

						// delete frame from follower
						follower->indexedFrames.erase(follower->indexedFrames.find(followerFrameIndex));
					}

					auto synchronisedFrame = std::make_shared<SynchronisedFrame>(
						cameraFrames
						, this->leaderID
					);

					synchronisedFrame->setComputeTime(std::chrono::high_resolution_clock::now() - startCompute);
					this->output.send(synchronisedFrame);

					// delete this frame from the leader
					itLeaderFrame = leader->indexedFrames.erase(itLeaderFrame);
				}
				else {
					itLeaderFrame++;
				}
			}
		}
		else {
			throw(Exception("This synchronisation strategy is not implemented"));
		}
	}
}