#pragma once
#include <map>
#include "MoCapLib.h"

#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	class Synchroniser
	{
	public:
		struct SyncMember {
			Links::Input input;
			std::map<uint64_t, std::shared_ptr<XimeaCameraFrame>> indexedFrames;
			std::weak_ptr<CameraThread> cameraThread;
		};
		Synchroniser();
		~Synchroniser();

		void checkConnections(const std::vector<Links::Output::ID>&);
		void requestResync();
	protected:
		void requestUpdate();

		void receiveAllFrames();
		void resync();

		bool joining = false;
		bool needsResync = true;

		Utils::WorkerThread workerThread;
		std::map<Links::Output::ID, std::unique_ptr<SyncMember>> syncMembers;
	};
}