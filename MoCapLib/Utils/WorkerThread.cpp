#include "pch_MoCapLib.h"
#include "WorkerThread.h"
#include "../Exception.h"

#include <iostream>

namespace TD_MoCap {
	namespace Utils {
#pragma mark PerformLock
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

#pragma mark WorkerThread
		//----------
		WorkerThread::WorkerThread()
		{
			this->thread = std::thread([this] {
				while (this->running) {
					Action action;
					if (workQueue.receive(action)) {
						action();
					}
				}
			});
		}

		//----------
		WorkerThread::~WorkerThread()
		{
			// join thread if user has not already joined it
			if (this->thread.joinable()) {
				this->join();
			}
		}

		//----------
		void
			WorkerThread::join()
		{
			this->running = false;
			this->workQueue.close();
			for (auto wakeable : this->wakeOnPerformBlocking) {
				wakeable->wake();
			}
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

			for (auto wakeable : this->wakeOnPerformBlocking) {
				wakeable->wake();
			}

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
			auto actionQueue = std::make_shared<ActionQueue>();
			auto performLock = std::shared_ptr<PerformLock>(new PerformLock(actionQueue));
			auto weakPerformLock = std::weak_ptr<PerformLock>(performLock);

			ThreadChannel<bool> enterPerformLockRegion;

			// Note
			// There are 2 mechanisms for retaining the perform lock:
			// 1. Checking if performLock is alive
			// 2. Checking that the actionQueue is still open

			this->perform([weakPerformLock, actionQueue, &enterPerformLockRegion] {
				// announce to acquitePerformLock thread that we've entered the perform lock region
				enterPerformLockRegion.send(true);

				// check that PerformLock is alive
				auto performLock = weakPerformLock.lock();
				while (performLock) {
					// allow the PerformLock to die elsewhere
					performLock.reset();
					
					// perform actions in queue
					Action action;
					while (actionQueue->receive(action)) {
						action();
					}

					performLock = weakPerformLock.lock(); 
				}
			});

			// Wait until the performLock region has been entered
			{
				bool enteredPerformLockRegion = false;
				enterPerformLockRegion.receive(enteredPerformLockRegion);
				while (!enteredPerformLockRegion) {
					enterPerformLockRegion.receive(enteredPerformLockRegion);
				}
			}

			return performLock;
		}

		//----------
		bool
			WorkerThread::isJoining() const
		{
			return !this->running;
		}

		//----------
		void
			WorkerThread::clearWorkItems()
		{
			this->workQueue.clear();
		}

		//----------
		size_t
			WorkerThread::sizeWorkItems() const
		{
			return this->workQueue.size();
		}
	}
}