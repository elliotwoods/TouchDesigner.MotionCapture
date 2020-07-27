#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "CentroidsFrame.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API TriangulateFrame : public BaseFrame
		{
		public:
			std::string getTypeName() const;
			uint64_t getFrameIndex() const;
		
			std::shared_ptr<CentroidsFrame> inputFrame;

			struct {
				std::vector<Math::Ray> raysLeft;
				std::vector<Math::Ray> raysRight;
				std::vector<Math::Ray> intersections;
			} testData;
		};
	}
}
