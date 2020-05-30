#include "pch_MoCapLib.h"
#include "Output.h"
#include "Input.h"

#include "../Utils/Table.h"

#include "OutputsRegister.h"

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

namespace TD_MoCap {
	namespace Links {
		//----------
		Output::ID Output::nextID = 0;

		//----------
		Output::Output()
		{
			OutputsRegister::X().add(this);
		}

		//----------
		Output::~Output()
		{
			std::lock_guard<std::mutex> lockSubscribers(this->lockSubscribers);
			for (const auto input : this->subscribedInputs)
			{
				input->unsubscribe(false);
			}

			OutputsRegister::X().remove(this);
		}

		//----------
		Output::ID
			Output::getID() const
		{
			return this->id;
		}

		//----------
		void
			Output::update()
		{
			{
				std::lock_guard<std::mutex> mutex(this->lockInfo);
				this->info.countThisFrame = this->incomingInfo.receivedCount;
				this->info.frameRate = this->incomingInfo.fps;
				this->info.computeTimeMs = this->incomingInfo.computeTimeMs;

				// reset the frame info
				this->incomingInfo.receivedCount = 0;
			}

			{
				std::lock_guard<std::mutex> lock(this->lockSubscribers);
				this->info.subscriberCount = this->subscribedInputs.size();
			}

			this->info.totalCount += this->info.countThisFrame;
		}

		//----------
		void
			Output::send(std::shared_ptr<BaseFrame> frame)
		{
			if(frame) {
				std::lock_guard<std::mutex> lockInfo(this->lockInfo);
				this->incomingInfo.receivedCount++;
				this->frameRateCounter.tick();
				this->incomingInfo.fps = this->frameRateCounter.getFPS();
				this->incomingInfo.computeTimeMs = (float) std::chrono::duration_cast<std::chrono::microseconds>(frame->getComputeTime()).count() / 1000.0f;

				std::lock_guard<std::mutex> lockSubscribers(this->lockSubscribers);
				for (auto input : this->subscribedInputs) {
					input->send(frame);
				}
			}
		}

		//----------
		void
			Output::populateMainThreadOutput(DAT_Output* output)
		{
			Utils::Table table;

			table.newRow() << "ID" << this->getID();
			table.newRow() << "framesSentThisMainloopFrame" << this->info.countThisFrame;
			table.newRow() << "totalFrameCount" << this->info.totalCount;
			table.newRow() << "outputFrameRate" << this->info.frameRate;
			table.newRow() << "computeTimeMs" << this->info.computeTimeMs;
			table.newRow() << "subscriberCount" << this->info.subscriberCount;

			table.populateOutput(output);
		}

		//----------
		bool
			Output::getIDFromDAT(const OP_DATInput* dat, Output::ID & result) {
			if (dat->numCols < 2 || dat->numRows < 1) {
				return false;
			}
			if (strcmp(dat->getCell(0, 0), "ID") != 0) {
				return false;
			}

			auto cellString = dat->getCell(0, 1);
			if (!is_number(cellString)) {
				return false;
			}

			result = strtol(cellString, nullptr, 10);
			return true;
		}

		//----------
		void
			Output::addSubscriber(Input* input)
		{
			std::lock_guard<std::mutex> lock(this->lockSubscribers);
			this->subscribedInputs.insert(input);
		}

		//----------
		void
			Output::removeSubscriber(Input* input)
		{
			std::lock_guard<std::mutex> lock(this->lockSubscribers);
			this->subscribedInputs.erase(input);
		}
	}
}