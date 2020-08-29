#include "pch_MoCapLib.h"
#include "BaseFrame.h"
#include "Exception.h"

namespace TD_MoCap {
	namespace Frames {
		//-----------
		std::set<BaseFrame*> BaseFrame::allFrames;
		std::mutex BaseFrame::lockAllFrames;

		//-----------
		BaseFrame::BaseFrame()
		{
			std::unique_lock<std::mutex> lock(BaseFrame::lockAllFrames);
			this->allFrames.insert(this);
		}

		//-----------
		BaseFrame::~BaseFrame() {
			std::unique_lock<std::mutex> lock(BaseFrame::lockAllFrames);
			this->allFrames.erase(this->allFrames.find(this));
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

		//-----------
		std::map<std::string, size_t>
			BaseFrame::getCountPerType()
		{
			std::unique_lock<std::mutex> lock(BaseFrame::lockAllFrames);
			std::map<std::string, size_t> countPerType;
			{
				for (const auto& frame : BaseFrame::allFrames) {
					auto typeName = frame->getTypeName();
					auto it = countPerType.find(typeName);
					if (it == countPerType.end()) {
						countPerType.emplace(typeName, 1);
					}
					else {
						it->second++;
					}
				}
			}
			return countPerType;
		}
	}
}