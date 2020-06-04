#pragma once
#include <map>
#include "MoCapLib.h"

#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	class Synchroniser
	{
	public:
		struct SyncMember
		{
			Links::Input input;
			std::map<uint64_t, std::shared_ptr<XimeaCameraFrame>> indexedFrames;

			uint64_t frameNumberStart = 0;
			std::chrono::microseconds timestampStart;
		};
		Synchroniser();
		~Synchroniser();

		void checkConnections(const std::vector<Links::Output::ID>&);
		void requestResync();
		void sendSynchronisedFrames();

		Utils::WorkerThread& getThread();
		Utils::ParameterList& getParameters();

		Links::Output output;
	protected:
		void requestUpdate();

		bool receiveAllFrames();
		void resync();

		bool needsResync = true;

		Utils::WorkerThread workerThread;

		std::mutex lockSyncMembers;
		std::map<Links::Output::ID, std::unique_ptr<SyncMember>> syncMembers;
		Links::Output::ID leaderID;

		const bool checkCameraTriggers = false;

		struct
		{
			Utils::NumberParameter<int> maxHistory
			{
				"History size", "frames"
				, 10, 10
				, 1, 10000
				, 1, 20
			};

			Utils::SelectorParameter strategy
			{
				"Strategy"
				, {"Frameindex", "Timestamp"}
				, "Frameindex"
			};

			Utils::NumberParameter<float> timestampWindow
			{
				"Timestamp window", "ms"
				, 1000.0f / 120.0f, 1000.0f / 120.0f
				, 0, 1000.0f
			};

			Utils::ParameterList list{ &maxHistory, &strategy, &timestampWindow};
		} parameters;
	};
}