#include "pch_OP_Player.h"
#include "Player.h"

namespace TD_MoCap {
	//----------
	Player::Player(const std::string& path, Links::Output& output, size_t threads, size_t buffer)
		: output(output)
	{
		this->path = std::filesystem::path(path);
		
		if (!std::filesystem::exists(this->path)) {
			throw(Exception("Folder does not exist"));
		}

		{
			auto jsonPath = this->path / "recording.json";
			if (!std::filesystem::exists(jsonPath)) {
				throw(Exception("recording.json not found in folder"));
			}

			// load json
			std::ifstream file(jsonPath.string());
			file >> this->playerState.recordingJson;
			file.close();
		}

		if (!this->playerState.recordingJson.contains("frames")) {
			throw(Exception("Invalid json - missing frames array"));
		}
		if (!this->playerState.recordingJson["frames"].is_array()) {
			throw(Exception("Invalid json - frames is not an array"));
		}

		this->playerState.frameCount = this->playerState.recordingJson["frames"].size();
		if (this->playerState.frameCount == 0) {
			throw(Exception("Recording is empty"));
		}

		this->playerState.lastFrameStart = std::chrono::high_resolution_clock::now();
		this->requestPlayFrame();
	}

	//----------
	Player::~Player()
	{
		this->thread.join();
	}

	//----------
	void
		Player::setFramerate(float framerate)
	{
		std::unique_lock<std::mutex> lock(this->lockPlayerState);
		this->playerState.framerate = framerate;
	}

	//----------
	void
		Player::update()
	{

	}

	//----------
	Utils::Table
		Player::getRecordingInfo()
	{
		std::unique_lock<std::mutex> lock(this->lockPlayerState);

		Utils::Table table;
		table.newRow() << "frameIndex" << this->playerState.frameIndex;
		table.newRow() << "frameCount" << this->playerState.frameCount;
		table.newRow() << "computer" << this->playerState.recordingJson["computer"];
		table.newRow() << "dateString" << this->playerState.recordingJson["dateString"];
		table.newRow() << "dateTimestamp" << this->playerState.recordingJson["dateTimestamp"];

		return table;
	}

	//----------
	void
		Player::requestPlayFrame()
	{
		if (this->thread.isJoining()) {
			return;
		}

		this->thread.perform([this] {
			this->lockPlayerState.lock();
			auto frameInterval = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::nanoseconds(1000000000) / this->playerState.framerate);
			auto lastFrameStart = this->playerState.lastFrameStart;
			this->lockPlayerState.unlock();

			std::this_thread::sleep_until(lastFrameStart + frameInterval);
			{
				// make a blank frame and start timer
				auto frame = std::make_shared<Frames::SynchronisedFrame>();
				frame->startComputeTimer();

				// record the time for next frame
				this->playerState.lastFrameStart = lastFrameStart + frameInterval; // use the ideal time so we dont drop fps

				// get the local player state
				std::unique_lock<std::mutex> lock(this->lockPlayerState);
				auto frameJson = this->playerState.recordingJson["frames"][this->playerState.frameIndex];

				// here we always make SynchronisedFrame - this could be extended to factory any frame type
				frame->deserialise(frameJson);

				// send the frame
				frame->endComputeTimer();
				this->output.send(frame);

				// advance to next frame
				this->playerState.frameIndex++;

				// loop
				this->playerState.frameIndex %= this->playerState.frameCount;
			}

			this->requestPlayFrame();
		});
	}
}
