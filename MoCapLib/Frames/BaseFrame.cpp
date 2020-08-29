#include "pch_MoCapLib.h"
#include "BaseFrame.h"
#include "Exception.h"

namespace TD_MoCap {
	namespace Frames {
		//-----------
		std::set<BaseFrame*> BaseFrame::allFrames;

		//-----------
		BaseFrame::BaseFrame()
		{
			this->allFrames.insert(this);
		}

		//-----------
		BaseFrame::~BaseFrame() {
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
		const std::set<BaseFrame*>&
			BaseFrame::getAllFrames()
		{
			return BaseFrame::allFrames;
		}
	}
}