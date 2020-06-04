#include "pch_MoCapLib.h"
#include "WorkerGroup.h"

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