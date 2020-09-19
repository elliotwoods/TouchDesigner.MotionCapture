#pragma once

#include "Utils/ThreadChannel.h"
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
			std::shared_ptr<Frames::BaseFrame> receiveNextFrameWait(const std::chrono::system_clock::duration& timeout);
			std::shared_ptr<Frames::BaseFrame> receiveNextFrameDontWait();

			// called from host class
			std::shared_ptr<Frames::BaseFrame> receiveLatestFrame(bool useCached);

			// called from host class
			std::shared_ptr<Frames::BaseFrame> getLastFrame();

			Utils::ThreadChannel<std::shared_ptr<Frames::BaseFrame>> & getChannel();

			size_t getDroppedFrameCount() const;
		protected:
			friend Output;

			// called from Links::Output
			void send(std::shared_ptr<Frames::BaseFrame>);

			Output* connectedOutput = nullptr;
			Utils::ThreadChannel<std::shared_ptr<Frames::BaseFrame>> channel;

			std::shared_ptr<Frames::BaseFrame> lastFrame;

			size_t maxSize = 120;
			size_t droppedFrameCount = 0;
		};
	}
}
