#include "pch_MoCapLib.h"
#include "TrackingFrame.h"


namespace TD_MoCap {
	namespace Frames {
		//----------
		std::shared_ptr<TrackingFrame>
			TrackingFrame::make()
		{
			return std::shared_ptr<TrackingFrame>(new TrackingFrame);
		}

		//----------
		std::string
			TrackingFrame::getTypeName() const
		{
			return "TrackingFrame";
		}

		//----------
		uint64_t
			TrackingFrame::getFrameIndex() const
		{
			return this->inputFrame->getFrameIndex();
		}

		//----------
		bool
			TrackingFrame::getPreviewCHOP(Utils::ChannelSet& channels) const
		{
			return false;
		}

		//----------
		bool
			TrackingFrame::getPreviewSOP(SOP_Output* output) const
		{
			for (auto& particle : this->trackedParticles) {
				auto& point = this->inputFrame->worldPoints[particle.incomingCentroidIndex];
				output->addPoint(* (Position*) &point);
			}
			return true;
		}

		//----------
		TrackingFrame::TrackingFrame()
		{

		}
	}
}
