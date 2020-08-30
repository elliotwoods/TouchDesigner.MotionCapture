#include "pch_MoCapLib.h"
#include "WorkerGroup.h"

#include <future>

namespace TD_MoCap {
	namespace Utils {
		//---------
		WorkerGroup::WorkerGroup()
		{
			for (size_t i = 0; i < std::thread::hardware_concurrency(); i++) {
				this->threads.emplace_back(new std::thread([this] {
					while (this->running) {
						std::shared_ptr<WorkerThread::Action> action;
						if (this->actionChannel.receive(action)) {
							try {
								rethrowFormattedExceptions([&action] {
									(*action)();
								});
							}
							catch (Exception e) {
								this->exceptionsInThread.send(e);
							}
						}
					}
				}));
			}
		}

		//---------
		WorkerGroup::~WorkerGroup()
		{
			this->close();
		}

		//---------
		WorkerGroup&
			WorkerGroup::X()
		{
			static std::unique_ptr<WorkerGroup> instance(new WorkerGroup());
			return *instance;
		}

		//---------
		std::weak_ptr<WorkerThread::Action>
			WorkerGroup::perform(const WorkerThread::Action& action)
		{
			auto sharedAction = std::make_shared<WorkerThread::Action>(action);
			this->actionChannel.send(sharedAction);

			return sharedAction;
		}

		//---------
		void
			WorkerGroup::parallelFor(const std::vector<WorkerThread::Action>& actions)
		{
			std::vector<std::shared_ptr<std::promise<void>>> promises;
			std::vector<std::future<void>> futures;

			ThreadChannel<Exception> exceptions;

			//load actions into queue
			for (const auto& action : actions) {
				promises.emplace_back(new std::promise<void>());
				auto promise = promises.back();
				futures.emplace_back(promise->get_future());

				this->perform([action, promise, &exceptions] {
					try {
						rethrowFormattedExceptions(action);
					}
					catch (Exception e) {
						exceptions.send(e);
					}
					promise->set_value();
				});
			}

			//wait for actions to complete
			for (auto& future : futures) {
				future.get();
			}

			// if there are any exceptions
			{
				Exception e;
				if (exceptions.tryReceive(e)) {
					// throw the first one
					throw(e);
				}
			}
			
		}

		//---------
		void
			WorkerGroup::clearWorkItems()
		{
			this->actionChannel.clear();
		}

		//---------
		size_t
			WorkerGroup::sizeWorkItems() const
		{
			return this->actionChannel.size();
		}

		//---------
		void
			WorkerGroup::close()
		{
			if (this->running) {
				this->running = false;
				this->actionChannel.close();
				for (const auto& thread : this->threads) {
					thread->join();
				}
			}
		}
	}
}