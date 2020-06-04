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

			void unsubscribe(bool informOutput = true);
			void subscribe(Output::ID);
			
			bool isConnected() const;

			// close the channel. release any blocking receives
			void close();

			// called from host class
			std::shared_ptr<BaseFrame> receiveNextFrameWait(const std::chrono::system_clock::duration& timeout);
			std::shared_ptr<BaseFrame> receiveNextFrameDontWait();

			// called from host class
			std::shared_ptr<BaseFrame> receiveLatestFrame(bool useCached);

			// called from host class
			std::shared_ptr<BaseFrame> getLastFrame();

			Utils::Channel<std::shared_ptr<BaseFrame>> & getChannel();
		protected:
			friend Output;

			// called from Links::Output
			void send(std::shared_ptr<BaseFrame>);

			Output* connectedOutput = nullptr;
			Utils::Channel<std::shared_ptr<BaseFrame>> channel;

			std::shared_ptr<BaseFrame> lastFrame;
		};
	}
}
