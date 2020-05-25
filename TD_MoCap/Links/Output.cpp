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
				this->info.receivedThisFrame = this->incomingInfo.receivedCount;

				// reset the frame info
				this->incomingInfo.receivedCount = 0;
			}
			this->info.totalReceivedFrames += this->info.receivedThisFrame;
		}

		//----------
		void Output::send(std::shared_ptr<BaseFrame> frame) {
			{
				std::lock_guard<std::mutex> mutex(this->lockInfo);
				this->incomingInfo.receivedCount++;
			}
		}

		//----------
		void Output::populateMainThreadOutput(DAT_Output* output) {
			Utils::Table table;

			table.newRow() << "ID" << this->info.ID;
			table.newRow() << "receivedThisFrame" << this->info.receivedThisFrame;
			table.newRow() << "totalReceivedFrames" << this->info.totalReceivedFrames;
			table.newRow() << "incomingFrameRate" << this->info.incomingFrameRate;
			table.newRow() << "subscribers" << this->info.subscribers;

			table.populateOutput(output);
		}
	}
}