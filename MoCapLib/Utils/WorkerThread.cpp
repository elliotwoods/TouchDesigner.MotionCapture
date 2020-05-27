#include "pch_MoCapLib.h"
#include "WorkerThread.h"
#include "../Exception.h"

#include <iostream>

namespace TD_MoCap {
	namespace Utils {
		//----------
		WorkerThread::PerformLock::~PerformLock()
		{
			this->actionQueue->close();
		}

		//----------
		void
			WorkerThread::PerformLock::perform(const Action& action)
		{
			this->actionQueue->send(action);
		}

		//----------
		WorkerThread::PerformLock::PerformLock(std::shared_ptr<ActionQueue> actionQueue)
		{
			this->actionQueue = actionQueue;
		}

		//----------
		WorkerThread::WorkerThread()
		{
			this->thread = std::thread([this] {
				while (this->running) {
					Action action;
					if (workQueue.tryReceive(action, 1000)) {
						action();
					}
				}
			});
		}

		//----------
		WorkerThread::~WorkerThread()
		{
			this->join();
		}

		//----------
		void
			WorkerThread::join()
		{
			this->running = false;
			this->workQueue.close();
			this->thread.join();
		}

		//----------
		void
			WorkerThread::perform(const Action& action)
		{
			workQueue.send([this, action] {
				try {
					rethrowFormattedExceptions(action);
				}
				catch (Exception& e) {
					this->exceptionsInThread.send(e);
				}
			});
		}

		//----------
		void
			WorkerThread::performBlocking(const Action& action)
		{
			std::mutex mutex;
			std::condition_variable cv;

			Exception exception;
			bool exceptionThrown = false;
			bool workComplete = false;

			this->perform([&] {

				{
					try {
						std::lock_guard<std::mutex> lock(mutex);
						action();
					}
					catch (const Exception& e) {
						exception = e;
						exceptionThrown = true;
					}
					catch (const std::exception& e) {
						exception = Exception(e.what());
						exceptionThrown = true;
					}
					catch (...) {
						exception = Exception("Unknown exception in performBlocking");
						exceptionThrown = true;
					}
				}

				workComplete = true;
				cv.notify_one();
			});

			std::unique_lock<std::mutex> lock(mutex);
			cv.wait(lock,
				[&workComplete] {
					return workComplete;
				});

			if (exceptionThrown) {
				throw(exception);
			}
		}

		//----------
		std::shared_ptr<WorkerThread::PerformLock>
			WorkerThread::acquirePerformLock()
		{
			auto actionChannel = std::make_shared<ActionQueue>();
			auto performLock = std::shared_ptr<PerformLock>(new PerformLock(actionChannel));
			auto weakPerformLock = std::weak_ptr<PerformLock>(performLock);

			this->perform([weakPerformLock, actionChannel] {
				// when performLock goes out of scope elsewhere, we no longer continue here
				auto performLock = weakPerformLock.lock();
				while (performLock) {
					performLock.reset();
					
					Action action;
					// the wait for receive is overriden by the close() called in the destructor of PerformLock
					while (actionChannel->receive(action)) {
						action();
					}

					performLock = weakPerformLock.lock(); 
				}
			});

			return performLock;
		}

		//----------
		bool
			WorkerThread::isJoining() const
		{
			return !this->running;
		}
	}
}