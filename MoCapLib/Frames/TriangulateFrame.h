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

			std::vector<Math::Ray> cameraLeftRays;
			std::vector<Math::Ray> cameraRightRays;
			std::vector<Math::Ray> intersections;
			std::vector<size_t> cameraLeftCentroidIndex;
			std::vector<size_t> cameraRightCentroidIndex;
			std::vector<glm::vec3> worldPoints;
		private:
			TriangulateFrame();
		};
	}
}
