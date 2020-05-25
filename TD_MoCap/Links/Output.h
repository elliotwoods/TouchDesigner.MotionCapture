#pragma once

#include "BaseFrame.h"
#include "Utils/FrameRateCounter.h"

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

			Utils::FrameRateCounter frameRateCounter;

			struct {
				uint32_t receivedCount = 0;
				float fps = 1.0f;
			} incomingInfo;

			struct {
				std::string ID = "TEST";
				uint32_t countThisFrame = 0;
				uint64_t totalCount = 0;
				float frameRate = 0;
				uint16_t subscriberCount = 0;
			} info;
		};
	}
}