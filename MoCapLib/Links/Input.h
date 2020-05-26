#pragma once

#include "Utils/Channel.h"
#include "Links/Output.h"

namespace TD_MoCap {
	namespace Links {
		class Output;

		class TDMOCAP_API Input
		{
		public:
			virtual ~Input();
			void update(const OP_DATInput*);

			void unsubscribe();
			void subscribe(Output::ID);
			
			// called from Links::Output
			void send(std::shared_ptr<BaseFrame>);

			// called from host class
			std::shared_ptr<BaseFrame> receiveNextFrame(bool waitForFrame);

			// called from host class
			std::shared_ptr<BaseFrame> receiveLatestFrame(bool useCached);
		protected:
			Output* connectedOutput = nullptr;
			Utils::Channel<std::shared_ptr<BaseFrame>> channel;

			std::shared_ptr<BaseFrame> lastFrame;
		};
	}
}
