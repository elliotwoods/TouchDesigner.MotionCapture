#pragma once

#include <thread>
#include <functional>
#include "Channel.h"
#include "Exception.h"

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API WorkerThread
		{
		public:
			typedef std::function<void()> Action;
			typedef Channel<Action> ActionQueue;

			class PerformLock {
			public:
				virtual ~PerformLock();
				void perform(const Action&);
				void performBlocking(const Action&);
			protected:
				friend WorkerThread;
				PerformLock(std::shared_ptr<ActionQueue>);
				std::shared_ptr<ActionQueue> actionQueue;
			};

			WorkerThread();
			~WorkerThread();
			void join();
			void perform(const Action&);
			void performBlocking(const Action&);
			bool isJoining() const;
			// Acquire a unique privilege to perform actions in this thread
			std::shared_ptr<PerformLock> acquirePerformLock();

			Channel<Exception> exceptionsInThread;
		protected:
			std::thread thread;
			Utils::Channel<std::function<void()>> workQueue;
			bool running = true;
		};
	}
}

