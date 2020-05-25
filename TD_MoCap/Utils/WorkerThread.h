#pragma once

#include <thread>
#include <functional>
#include "Channel.h"

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API WorkerThread
		{
		public:
			WorkerThread();
			~WorkerThread();
			void join();
			void perform(const std::function<void()>&);
			void performBlocking(const std::function<void()>&);
		protected:
			std::thread thread;
			Utils::Channel<std::function<void()>> workQueue;
			bool running = true;
		};
	}
}

