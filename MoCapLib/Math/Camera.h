#pragma once

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>

namespace TD_MoCap {
	namespace Math {
		class Camera {
		public:
			void setIntrinsics(const cv::Mat& cameraMatrix, const cv::Mat& distortionCoefficients);
			void setExtrinsics(const cv::Mat& rotationMatrix, const cv::Mat& translation);
		};
	}
}