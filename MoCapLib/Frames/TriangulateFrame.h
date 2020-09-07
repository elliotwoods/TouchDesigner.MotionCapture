#pragma once

#include <glm/glm.hpp>

#include "BaseFrame.h"
#include "CentroidsFrame.h"
#include "Math/Ray.h"
#include "Math/Camera.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API TriangulateFrame : public BaseFrame
		{
		public:
			static std::shared_ptr<TriangulateFrame> make();

			std::string getTypeName() const;
			uint64_t getFrameIndex() const;
		
			bool getPreviewCHOP(Utils::ChannelSet&) const override;
			bool getPreviewSOP(SOP_Output*) const override;
			std::shared_ptr<CentroidsFrame> inputFrame;

			std::vector<Math::Ray> cameraLeftRays;
			std::vector<Math::Ray> cameraRightRays;
			std::vector<Math::Ray> intersections;
			std::vector<cv::Point2f> cameraLeftCentroids;
			std::vector<cv::Point2f> cameraLeftCentroidsRaw;
			std::vector<cv::Point2f> cameraRightCentroids;
			std::vector<cv::Point2f> cameraRightCentroidsRaw;
			std::vector<float> cameraLeftMasses;
			std::vector<glm::vec3> worldPoints;
			std::vector<float> epipolarDistance;
			std::vector<float> massRatio;
			std::vector<float> angleDistance;

			Math::Camera cameraLeft, cameraRight;
		private:
			TriangulateFrame();
		};
	}
}
