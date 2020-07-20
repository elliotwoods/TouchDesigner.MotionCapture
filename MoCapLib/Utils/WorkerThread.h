#pragma once

#include <thread>
#include <functional>
#include "Channel.h"
#include "Exception.h"
#include "Wakeable.h"

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

			void clearWorkItems();
			size_t sizeWorkItems() const;

			// A set of objects (e.g. Channel's) to wake up whenever a performBlocking call occurs
			//
			// Be sure to remove all invalid members before disposing this class
			// e.g. make sure to manually call `join()` on this class when using this feature
			std::set<Wakeable *> wakeOnPerformBlocking;

			// Acquire a unique privilege to perform actions in this thread for the lifetime of the returned PerformLock
			std::shared_ptr<PerformLock> acquirePerformLock();

			Channel<Exception> exceptionsInThread;
		protected:
			std::thread thread;
			Utils::Channel<std::function<void()>> workQueue;
			bool running = true;
		};
	}
}

