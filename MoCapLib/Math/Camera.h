#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "Ray.h"

namespace TD_MoCap {
	namespace Math {
		class TDMOCAP_API Camera {
		public:
			Camera();
			void load(const std::filesystem::path&);

			void setIntrinsics(const cv::Mat& cameraMatrix, const cv::Mat& distortionCoefficients, const cv::Size& size);
			void setExtrinsics(const cv::Mat& rotationMatrix, const cv::Mat& translation);

			const cv::Mat & getCameraMatrix() const;
			const cv::Mat& getDistortionCoefficients() const;

			bool empty() const;

			glm::vec3 getPosition() const;
			glm::mat4 getProjectionMatrix() const;
			glm::mat4 getClippedProjectionMatrix() const;
			glm::mat4 getViewMatrix() const;
			glm::mat4 getInverseViewProjection() const;

			std::vector<cv::Point2f> undistortImagePoints(const std::vector<cv::Point2f>&, size_t roiY) const;
			std::vector<Ray> unprojectUndistortedImagePoints(const std::vector<cv::Point2f>&) const;
			Ray unprojectCoordinate(const cv::Point2f& imageCoordinate) const;

			std::vector<cv::Point2f> undistortedImagePointsToCoordinates(const std::vector<cv::Point2f>& imagePoints) const;
		protected:
			void updateProjectionMatrix();
			void updateViewMatrix();

			cv::Mat cameraMatrix;
			cv::Mat distortionCoefficients;
			cv::Size size;

			cv::Mat rotationMatrix, translation;

			glm::mat4 projectionMatrix;
			glm::mat4 clippedProjectionMatrix;

			glm::mat4 viewMatrix;
			glm::vec3 position;

			glm::mat4 inverseViewProjection;
		};
	}
}