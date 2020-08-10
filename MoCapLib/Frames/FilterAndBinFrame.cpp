#include "pch_MoCapLib.h"
#include "FilterAndBinFrame.h"

namespace TD_MoCap {
	namespace Frames {
		//----------
		std::shared_ptr<FilterAndBinFrame>
			FilterAndBinFrame::make()
		{
			return std::shared_ptr<FilterAndBinFrame>(new FilterAndBinFrame);
		}

		//----------
		std::string
			FilterAndBinFrame::getTypeName() const
		{
			return "FilterAndBinFrame";
		}

		//----------
		uint64_t
			FilterAndBinFrame::getFrameIndex() const
		{
			return this->inputFrame->getFrameIndex();
		}

		//----------
		bool
			FilterAndBinFrame::getPreviewCHOP(Utils::ChannelSet& channelSet) const
		{
			channelSet.setChannels({ "Alive", "x", "y", "z", "id", "lifetime"});
			channelSet.setSampleCount(this->particleBins.size());

			for (size_t i = 0; i < this->particleBins.size(); i++) {
				auto c = 0;
				if (particleBins[i] == -1) {
					channelSet[c++].samples[i] = 0;

					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;

					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;
				}
				else {
					channelSet[c++].samples[i] = 1;

					const auto& worldPoint = this->inputFrame->inputFrame->worldPoints[this->particleBins[i]];
					channelSet[c++].samples[i] = worldPoint.x;
					channelSet[c++].samples[i] = worldPoint.y;
					channelSet[c++].samples[i] = worldPoint.z;

					channelSet[c++].samples[i] = this->ids[i];
					channelSet[c++].samples[i] = this->inputFrame->trackedParticles[this->particleBins[i]].lifeTime;
				}
			}
			return true;
		}

		//----------
		bool
			FilterAndBinFrame::getPreviewSOP(SOP_Output* output) const
		{
			int32_t vertexIndex = 0;
			for (auto& particleIndex : this->particleBins) {
				if (particleIndex != -1) {
					const auto& worldPointNow = this->inputFrame->inputFrame->worldPoints[particleIndex];
					const auto& worldPointPriorIndex = this->inputFrame->trackedParticles[particleIndex].priorTriangulatedParticleIndex;
					const auto& worldPointPrior = this->previousFrame->inputFrame->inputFrame->worldPoints[worldPointPriorIndex];

					output->addPoint(*(Position*)&worldPointNow);
					output->addPoint(*(Position*)&worldPointPrior);

					int32_t lineIndices[2] = { vertexIndex++, vertexIndex++ };
					output->addLine(lineIndices , 2);
				}
			}
			return true;
		}

		//----------
		FilterAndBinFrame::FilterAndBinFrame()
		{

		}
	}
}