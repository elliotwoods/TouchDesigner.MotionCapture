#pragma once

#include "Frames/BaseFrame.h"
#include "Utils/FrameRateCounter.h"

#include <memory>
#include <mutex>

namespace TD_MoCap {
	namespace Links {
		class Input;

		class TDMOCAP_API Output
		{
		public:
			typedef uint16_t ID;

			Output();
			virtual ~Output();

			ID getID() const;

			void update();
			void send(std::shared_ptr<Frames::BaseFrame>);
			void populateMainThreadOutput(DAT_Output*);

			static bool getIDFromDAT(const OP_DATInput*, Output::ID& result);
		protected:
			friend Input;
			void addSubscriber(Input*);
			void removeSubscriber(Input*);

			static ID nextID;
			ID id = nextID++;

			std::mutex lockSubscribers;
			std::set<Input*> subscribedInputs;

			std::mutex lockInfo;

			Utils::FrameRateCounter frameRateCounter;

			struct {
				uint32_t receivedCount = 0;
				float fps = 0.0f;
				float fpsUnfiltered = 0.0f;
				float computeTimeMs = 0;
				std::string frameType;
			} incomingInfo;

			struct {
				uint32_t countThisFrame = 0;
				uint64_t totalCount = 0;
				float frameRate = 0;
				float frameRateUnfiltered = 0;
				float computeTimeMs = 0;
				std::string frameType;
				uint16_t subscriberCount = 0;
			} info;
		};
	}
}