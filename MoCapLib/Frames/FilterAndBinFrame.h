#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "TrackingFrame.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API FilterAndBinFrame : public BaseFrame
		{
		public:
			struct ParticleBin {
				bool occupied = false;
				bool newBinAssignment = false;
				size_t currentIndex; // or index at death
				glm::vec3 position;
				glm::vec3 frameVelocity;
				size_t lifetime;

				bool fullyAlive = true; // always check occupied also
				size_t afterlifeDuration = 0;
				bool resurrected = false;
			};

			static std::shared_ptr<FilterAndBinFrame> make();

			std::string getTypeName() const;
			uint64_t getFrameIndex() const;

			bool getPreviewCHOP(Utils::ChannelSet&) const override;
			bool getPreviewSOP(SOP_Output*) const override;

			std::shared_ptr<TrackingFrame> inputFrame;

			std::vector<ParticleBin> particleBins; // triangulated indices
		private:
			FilterAndBinFrame();
		};
	}
}