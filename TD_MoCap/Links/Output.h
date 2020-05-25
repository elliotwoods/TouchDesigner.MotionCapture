#pragma once

#include "BaseFrame.h"

#include <memory>
#include <mutex>

namespace TD_MoCap {
	namespace Links {

		class TDMOCAP_API Output
		{
		public:
			Output();

			void update();
			void send(std::shared_ptr<BaseFrame>);
			void populateMainThreadOutput(DAT_Output*);
		protected:
			std::mutex lockInfo;

			struct {
				uint32_t receivedCount = 0;
			} incomingInfo;

			struct {
				std::string ID = "TEST";
				uint32_t receivedThisFrame = 0;
				uint64_t totalReceivedFrames = 0;
				float incomingFrameRate = 0;
				uint16_t subscribers = 0;
			} info;
		};
	}
}