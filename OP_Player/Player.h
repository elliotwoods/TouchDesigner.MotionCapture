#pragma once

#include "MoCapLib.h"

#include <atomic>

namespace TD_MoCap {
	class Player {
	public:
		struct RecordingInfo {
			uint64_t frameIndex;
			uint64_t frameCount;
			std::string computerName;
			std::string dateString;
			std::string dateTimestamp;
		};

		Player(const std::string& path
			, Links::Output & output
			, size_t threads
			, size_t buffer);
		~Player();

		void setFramerate(float);
		void update();
		Utils::Table getRecordingInfo();
	protected:
		void requestPlayFrame();

		Links::Output & output;

		Utils::WorkerThread thread;

		
		std::mutex lockPlayerState;
		struct {
			std::filesystem::path path;
			nlohmann::json recordingJson;
			float framerate = 50.0f; // needs a default for first frame
			uint64_t frameIndex;
			uint64_t frameCount;
			std::chrono::high_resolution_clock::time_point lastFrameStart;
		} playerState;
	};
}