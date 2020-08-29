#include "pch_MoCapLib.h"
#include "BaseFrame.h"
#include "Exception.h"

namespace TD_MoCap {
	namespace Frames {
		//-----------
		std::map<std::string, size_t> BaseFrame::countPerType;

		//-----------
		BaseFrame::BaseFrame()
		{
			const auto typeName = this->getTypeName();
			auto findCount = BaseFrame::countPerType.find(typeName);
			if (findCount == BaseFrame::countPerType.end()) {
				BaseFrame::countPerType.emplace(typeName, 1);
			}
			else {
				findCount->second++;
			}
		}

		//-----------
		BaseFrame::~BaseFrame() {
			BaseFrame::countPerType[this->getTypeName()]--;
		}

		//-----------
		std::string
			BaseFrame::getTypeName() const
		{
			throw(Exception("Cannot get type of BaseFrame"));
		}

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