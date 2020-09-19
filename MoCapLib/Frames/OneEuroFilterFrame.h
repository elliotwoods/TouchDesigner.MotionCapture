#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "FilterAndBinFrame.h"

#include "../Utils/OneEuroFilter.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API OneEuroFilterFrame : public BaseFrame
		{
		public:
			struct TDMOCAP_API ParticleBin {
				ParticleBin(size_t UID, glm::vec3 position, const Utils::OneEuroFilter::Params &, size_t lifetime);
				size_t UID;
				glm::vec3 filteredPosition;
				glm::vec3 rawPosition;
				bool isNew;
				size_t lifetime;
				Utils::OneEuroFilter filter[3];
			};

			static std::shared_ptr<OneEuroFilterFrame> make();

			std::string getTypeName() const;
			uint64_t getFrameIndex() const;

			bool getPreviewCHOP(Utils::ChannelSet&) const override;
			bool getPreviewSOP(SOP_Output*) const override;

			std::shared_ptr<FilterAndBinFrame> inputFrame;

			std::vector<std::shared_ptr<ParticleBin>> particleBins;
		private:
			OneEuroFilterFrame();
		};
	}
}