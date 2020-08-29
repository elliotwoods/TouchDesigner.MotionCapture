#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "TrackingFrame.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API FilterAndBinFrame : public BaseFrame
		{
		public:
			static std::shared_ptr<FilterAndBinFrame> make();

			std::string getTypeName() const;
			uint64_t getFrameIndex() const;

			bool getPreviewCHOP(Utils::ChannelSet&) const override;
			bool getPreviewSOP(SOP_Output*) const override;

			std::shared_ptr<TrackingFrame> inputFrame;

			std::vector<int32_t> particleBins; // triangulated indices
			std::vector<size_t> ids;
		private:
			FilterAndBinFrame();
		};
	}
}