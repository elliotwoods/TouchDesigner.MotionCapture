#include "pch_MoCapLib.h"
#include "BaseFrame.h"

namespace TD_MoCap {
	namespace Links {
		//-----------
		void
			BaseFrame::startComputeTimer()
		{
			this->startCompute = std::chrono::high_resolution_clock::now();
		}

		//-----------
		void
			BaseFrame::endComputeTimer()
		{
			this->computeTime = std::chrono::high_resolution_clock::now() - this->startCompute;
		}
		
		//-----------
		void
			BaseFrame::setComputeTime(const std::chrono::high_resolution_clock::duration& computeTime)
		{
			this->computeTime = computeTime;
		}

		//-----------
		std::chrono::high_resolution_clock::duration
			BaseFrame::getComputeTime() const
		{
			return this->computeTime;
		}
	}
}