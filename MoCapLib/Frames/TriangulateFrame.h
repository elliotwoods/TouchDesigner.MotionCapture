#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "CentroidsFrame.h"
#include "Math/Ray.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API TriangulateFrame : public BaseFrame
		{
		public:
			static std::shared_ptr<TriangulateFrame> make();
			std::string getTypeName() const;
			uint64_t getFrameIndex() const;
		
			bool getPreviewCHOP(Utils::ChannelSet&) const override;

			std::shared_ptr<CentroidsFrame> inputFrame;

			struct {
				// Note : these should all be the same size! (for preview output)
				std::vector<Math::Ray> raysLeft;
				std::vector<Math::Ray> raysRight;
				std::vector<Math::Ray> intersections;
			} testData;
		private:
			TriangulateFrame();
		};
	}
}
