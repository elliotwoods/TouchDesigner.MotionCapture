#include "pch_MoCapLib.h"
#include "WorkerThread.h"
#include "../Exception.h"

#include <iostream>

namespace TD_MoCap {
	namespace Utils {
		//----------
		WorkerThread::WorkerThread()
		{
			this->thread = std::thread([this] {
				while (this->running) {
					std::function<void()> action;
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
			this->perform([] {}); // send an empty action to nudge it out of waiting
			this->thread.join();
		}

		//----------
		void
			WorkerThread::perform(const std::function<void()>& action)
		{
			this->workQueue.send(action);
		}

		//----------
		void
			WorkerThread::performBlocking(const std::function<void()>& action)
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
	}
}