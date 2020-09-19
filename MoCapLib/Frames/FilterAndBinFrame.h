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
				bool occupied;
				bool newBinAssignment;
				size_t UID;
				size_t currentTriangulatedIndex; // or index at death
				glm::vec3 position;
				glm::vec3 frameVelocity;
				glm::vec3 frameVelocityFiltered;
				size_t lifetime;

				bool fullyAlive; // always check occupied also
				size_t afterlifeDuration;
				size_t resurrectionCount;
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