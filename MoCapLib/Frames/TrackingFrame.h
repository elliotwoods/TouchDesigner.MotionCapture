#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "TriangulateFrame.h"

typedef uint16_t QTPoint;

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API TrackingFrame : public BaseFrame
		{
		public:
			struct Particle {
				size_t priorTriangulatedParticleIndex;
				glm::vec3 triangulatedParticlePosition;
				glm::vec3 priorTriangulatedParticlePosition;
				size_t lifeTime = 1;
			};

			static std::shared_ptr<TrackingFrame> make();

			std::string getTypeName() const;
			uint64_t getFrameIndex() const;

			bool getPreviewCHOP(Utils::ChannelSet&) const override;
			bool getPreviewSOP(SOP_Output*) const override;

			std::shared_ptr<TriangulateFrame> inputFrame;

			std::map<size_t, Particle> trackedParticles; // by current triangulated particle index
		private:
			TrackingFrame();
		};
	}
}
