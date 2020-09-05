#include "pch_OP_Player.h"
#include "Player.h"

namespace TD_MoCap {
	//----------
	Player::Player(const std::string& path, Links::Output& output)
		: output(output)
	{
		if (path.empty()) {
			throw(Exception("Folder not specified"));
		}

		this->playerState.path = std::filesystem::path(path);

		if (!std::filesystem::exists(this->playerState.path)) {
			throw(Exception("Folder does not exist"));
		}

		{
			auto jsonPath = this->playerState.path / "recording.json";
			if (!std::filesystem::exists(jsonPath)) {
				throw(Exception("recording.json not found in folder"));
			}

			// load json
			std::ifstream file(jsonPath.string());
			auto json = std::make_shared<nlohmann::json>();
			file >> (*json);
			this->playerState.recordingJson = json;
			file.close();
		}

		if (!this->playerState.recordingJson->contains("frames")) {
			throw(Exception("Invalid json - missing frames array"));
		}
		if (!this->playerState.recordingJson->at("frames").is_array()) {
			throw(Exception("Invalid json - frames is not an array"));
		}

		this->playerState.frameCount = this->playerState.recordingJson->at("frames").size();
		if (this->playerState.frameCount == 0) {
			throw(Exception("Recording is empty"));
		}

		this->playerState.lastFrameStart = std::chrono::high_resolution_clock::now();

		this->thread.setIdleFunction([this] {
			this->threadedUpdate();
		});
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
		if (framerate != this->playerState.framerate) {
			this->playerState.framerate = framerate;
			this->playerState.lastFrameStart = std::chrono::high_resolution_clock::now();
		}
	}

	//----------
	void
		Player::setHoldFrame(bool holdFrame)
	{
		std::unique_lock<std::mutex> lock(this->lockPlayerState);
		if (holdFrame != this->playerState.holdFrame) {
			this->playerState.holdFrame = holdFrame;
		}
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
		table.newRow() << "computer" << this->playerState.recordingJson->at("computer");
		table.newRow() << "dateString" << this->playerState.recordingJson->at("dateString");
		table.newRow() << "dateTimestamp" << this->playerState.recordingJson->at("dateTimestamp");

		return table;
	}

	//----------
	void
		Player::threadedUpdate()
	{
		// load information out of playerState
		this->lockPlayerState.lock();
		auto frameInterval = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::nanoseconds(1000000000) / this->playerState.framerate);
		auto playerStateCopy = this->playerState;
		this->lockPlayerState.unlock();

		// sleep until next frame should be presented
		std::this_thread::sleep_until(playerStateCopy.lastFrameStart + frameInterval);

		// create the frame and send it
		auto frame = Frames::SynchronisedFrame::make(this->playerState.recordingJson->at("frames")[playerStateCopy.frameIndex]
		, playerState.path);
		
		// record the time for next frame
		auto newLastFrameStart = playerStateCopy.lastFrameStart + frameInterval; // use the ideal time so we dont drop fps

		this->output.send(std::move(frame));

		// Update the player state
		{
			std::unique_lock<std::mutex> lock(this->lockPlayerState);
			// advance to next frame
			if (!this->playerState.holdFrame) {
				this->playerState.frameIndex++;
			}

			// loop
			this->playerState.frameIndex %= this->playerState.frameCount;

			this->playerState.lastFrameStart = newLastFrameStart;
		}
	}
}
