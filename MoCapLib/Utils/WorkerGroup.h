#pragma once

#include "Channel.h"
#include "WorkerThread.h"

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API WorkerGroup
		{
		public:
			WorkerGroup();
			virtual ~WorkerGroup();

			static WorkerGroup& X();
			std::weak_ptr<WorkerThread::Action> perform(const WorkerThread::Action&);

			void clearWorkItems();
			size_t sizeWorkItems() const;
			
			void close();

			Channel<Exception> errorsInThreads;
		protected:
			std::vector<std::unique_ptr<std::thread>> threads;
			Channel<std::shared_ptr<WorkerThread::Action>> actionChannel;
			bool running = true;
		};
	}
}