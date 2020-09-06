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
			channelSet.setChannels({ "Alive", "New"
				, "x", "y", "z"
				, "vx", "vy", "vz"
				, "id", "lifetime"});
			channelSet.setSampleCount(this->particleBins.size());

			for (size_t i = 0; i < this->particleBins.size(); i++) {
				auto c = 0;
				if (!particleBins[i].occupied) {
					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;

					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;

					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;

					channelSet[c++].samples[i] = 0;
					channelSet[c++].samples[i] = 0;
				}
				else {
					channelSet[c++].samples[i] = 1;

					channelSet[c++].samples[i] = this->particleBins[i].newBinAssignment;


					{
						const auto& worldPoint = this->particleBins[i].position;
						channelSet[c++].samples[i] = worldPoint.x;
						channelSet[c++].samples[i] = worldPoint.y;
						channelSet[c++].samples[i] = worldPoint.z;
					}

					{
						const auto& velocity = this->particleBins[i].frameVelocity;
						channelSet[c++].samples[i] = velocity.x;
						channelSet[c++].samples[i] = velocity.y;
						channelSet[c++].samples[i] = velocity.z;
					}

					channelSet[c++].samples[i] = this->particleBins[i].currentTriangulatedIndex;
					channelSet[c++].samples[i] = this->particleBins[i].lifetime;
				}
			}
			return true;
		}

		//----------
		bool
			FilterAndBinFrame::getPreviewSOP(SOP_Output* output) const
		{
			int32_t vertexIndex = 0;
			for (const auto& particleBin : this->particleBins) {
				if (particleBin.occupied) {
					const auto& worldPointNow = particleBin.position;
					const auto& worldPointPrior = particleBin.position - particleBin.frameVelocity;

					output->addPoint(*(Position*)&worldPointNow);
					output->addPoint(*(Position*)&worldPointPrior);
				}
				else {
					output->addPoint(Position{ 0, 0, 0 });
					output->addPoint(Position{ 0, 0, 0 });
				}

				int32_t lineIndices[2] = { vertexIndex++, vertexIndex++ };
				output->addLine(lineIndices, 2);
			}
			return true;
		}

		//----------
		FilterAndBinFrame::FilterAndBinFrame()
		{

		}
	}
}