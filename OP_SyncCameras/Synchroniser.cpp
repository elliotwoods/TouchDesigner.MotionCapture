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
	void Synchroniser::receiveAllFrames()
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
	}
}
