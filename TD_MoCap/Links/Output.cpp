#include "pch_TD_MoCap.h"
#include "Output.h"
#include "../Utils/Table.h"

namespace TD_MoCap {
	namespace Links {
		//----------
		Output::Output() {
		}

		//----------
		void Output::update() {
			{
				std::lock_guard<std::mutex> mutex(this->lockInfo);
				this->info.countThisFrame = this->incomingInfo.receivedCount;
				this->info.frameRate = this->incomingInfo.fps;

				// reset the frame info
				this->incomingInfo.receivedCount = 0;
			}
			this->info.totalCount += this->info.countThisFrame;
		}

		//----------
		void Output::send(std::shared_ptr<BaseFrame> frame) {
			{
				std::lock_guard<std::mutex> mutex(this->lockInfo);
				this->incomingInfo.receivedCount++;
				this->frameRateCounter.tick();
				this->incomingInfo.fps = this->frameRateCounter.getFPS();
			}
		}

		//----------
		void Output::populateMainThreadOutput(DAT_Output* output) {
			Utils::Table table;

			table.newRow() << "ID" << this->info.ID;
			table.newRow() << "framesSentThisMainloopFrame" << this->info.countThisFrame;
			table.newRow() << "totalFrameCount" << this->info.totalCount;
			table.newRow() << "outputFrameRate" << this->info.frameRate;
			table.newRow() << "subscriberCount" << this->info.subscriberCount;

			table.populateOutput(output);
		}
	}
}