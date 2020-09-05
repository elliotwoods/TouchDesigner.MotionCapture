#include "pch_MoCapLib.h"
#include "OneEuroFilterFrame.h"

namespace TD_MoCap {
	namespace Frames {
		//----------
		OneEuroFilterFrame::ParticleBin::ParticleBin(glm::vec3 position, const Utils::OneEuroFilter::Params& params, size_t lifetime)
			: rawPosition(position)
			, filteredPosition(position)
			, filter{
				params
				, params
				, params
			}
			, lifetime(lifetime)
			, isNew(true)
		{

		}

		//----------
		std::shared_ptr<OneEuroFilterFrame>
			OneEuroFilterFrame::make()
		{
			return std::shared_ptr<OneEuroFilterFrame>(new OneEuroFilterFrame);
		}

		//----------
		std::string
			OneEuroFilterFrame::getTypeName() const
		{
			return "OneEuroFilterFrame";
		}

		//----------
		uint64_t
			OneEuroFilterFrame::getFrameIndex() const
		{
			return this->inputFrame->getFrameIndex();
		}

		//----------
		bool
			OneEuroFilterFrame::getPreviewCHOP(Utils::ChannelSet& channelSet) const
		{
			channelSet.setChannels({ "Alive", "New"
				, "x", "y", "z"
				, "raw_x", "raw_y", "raw_z"
				, "lifetime" });
			channelSet.setSampleCount(this->particleBins.size());

			for (size_t i = 0; i < this->particleBins.size(); i++) {
				auto c = 0;
				if (!this->particleBins[i]) {
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
					const auto& particle = * this->particleBins[i];

					channelSet[c++].samples[i] = 1;
					channelSet[c++].samples[i] = particle.isNew;

					channelSet[c++].samples[i] = particle.filteredPosition[0];
					channelSet[c++].samples[i] = particle.filteredPosition[1];
					channelSet[c++].samples[i] = particle.filteredPosition[2];

					channelSet[c++].samples[i] = particle.rawPosition[0];
					channelSet[c++].samples[i] = particle.rawPosition[1];
					channelSet[c++].samples[i] = particle.rawPosition[2];

					channelSet[c++].samples[i] = particle.lifetime;
				}
			}

			return true;
		}

		//----------
		bool
			OneEuroFilterFrame::getPreviewSOP(SOP_Output* output) const
		{
			int32_t vertexIndex = 0;
			for (const auto& particleBin : this->particleBins) {
				if (particleBin) {
					const auto& worldPoint = particleBin->filteredPosition;
				
					output->addPoint(*(Position*)&worldPoint);
				}
				else {
					output->addPoint(Position{ 0, 0, 0 });
				}
			}
			return true;
		}

		//----------
		OneEuroFilterFrame::OneEuroFilterFrame()
		{

		}
	}
}
