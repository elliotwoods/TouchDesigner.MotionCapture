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

			uint64_t frameNumberStart = 0;
			std::chrono::microseconds timestampStart;
		};
		Synchroniser();
		~Synchroniser();

		void checkConnections(const std::vector<Links::Output::ID>&);
		void requestResync();

		Utils::WorkerThread& getThread();
	protected:
		void requestUpdate();

		void receiveAllFrames();
		void resync();

		bool needsResync = true;

		Utils::WorkerThread workerThread;
		std::map<Links::Output::ID, std::unique_ptr<SyncMember>> syncMembers;
		
		const bool checkCameraTriggers = false;
	};
}