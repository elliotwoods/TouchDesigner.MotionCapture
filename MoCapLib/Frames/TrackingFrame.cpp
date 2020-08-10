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
			channels.setChannels({ "x", "y", "z", "lifetime" });
			channels.setSampleCount(this->trackedParticles.size());
			size_t i = 0;
			for (const auto & trackedParticle : this->trackedParticles) {

				auto c = 0;
				auto worldPoint = this->inputFrame->worldPoints[trackedParticle.first];
				channels[c++].samples[i] = worldPoint.x;
				channels[c++].samples[i] = worldPoint.y;
				channels[c++].samples[i] = worldPoint.z;
				channels[c++].samples[i] = trackedParticle.second.lifeTime;

				i++;
			}

			return true;
		}

		//----------
		bool
			TrackingFrame::getPreviewSOP(SOP_Output* output) const
		{
			for (const auto& particle : this->trackedParticles) {
				auto& point = this->inputFrame->worldPoints[particle.first];
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
