#include "pch_MoCapLib.h"
#include "Output.h"
#include "Input.h"

#include "../Utils/Table.h"

#include "OutputsRegister.h"

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
			for (const auto input : this->subscribedInputs)
			{
				input->unsubscribe();
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
			{
				std::lock_guard<std::mutex> mutex(this->lockInfo);
				this->incomingInfo.receivedCount++;
				this->frameRateCounter.tick();
				this->incomingInfo.fps = this->frameRateCounter.getFPS();

				for (auto input : this->subscribedInputs) {
					input->channel.send(frame);
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
			table.newRow() << "subscriberCount" << this->info.subscriberCount;

			table.populateOutput(output);
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