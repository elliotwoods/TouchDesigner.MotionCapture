#include "pch_OP_Triangulate.h"
#include "Process_Triangulate.h"

#include "testData.h"

namespace TD_MoCap {
#pragma mark TriangulateParameters
	//----------
	void
		TriangulateParameters::update()
	{
		// left camera intrinsics
		{
			auto path = this->leftCameraCalibration.getValue();
			if (!path.empty()) {
				this->cameraLeft.load(path);
			}
		}

		// right camera intrinsics
		{
			auto path = this->rightCameraCalibration.getValue();
			if (!path.empty()) {
				this->cameraRight.load(path);
			}
		}

		// stereo camera calibration
		{
			auto path = this->stereoCameraCalibration.getValue();
			if (!path.empty()) {
				cv::FileStorage fs(this->stereoCameraCalibration.getValue(), cv::FileStorage::READ);
				if (!fs.isOpened()) {
					throw(Exception("Failed to open stereo calibration"));
				}
				cv::Mat rotationMatrix, translation;
				fs["rotation"] >> rotationMatrix;
				fs["translation"] >> translation;
				fs["fundamental_matrix"] >> this->fundamental_matrix;
				this->fundamental_matrix.convertTo(this->fundamental_matrix, CV_32F);
				this->cameraRight.setExtrinsics(rotationMatrix, translation);

				// left camera in right
				{
					auto zeroPositionAsArray = std::vector<cv::Point3f>(1, { 0, 0, 0 });

					std::vector<cv::Point2f> projectedImagePoint;
					cv::projectPoints(zeroPositionAsArray
						, rotationMatrix
						, translation
						, this->cameraRight.getCameraMatrix()
						, cv::Mat() // we work in undistorted space
						, projectedImagePoint);
					this->cameraLeftInCameraRight = projectedImagePoint[0];
				}
			}
		}
	}

#pragma mark Process_Triangulate
	//----------
	void
		Process_Triangulate::process(std::shared_ptr<Frames::CentroidsFrame> inputFrame
			, std::shared_ptr<Frames::TriangulateFrame> outputFrame
			, TriangulateParameters& parameters)
	{
		if (inputFrame->cameras.size() != 2) {
			throw(Exception("Triangulation currently only supports exactly 2 cameras"));
		}

		auto leftCamera = inputFrame->cameras[inputFrame->inputFrame->leaderID];
		auto rightCamera = inputFrame->cameras[inputFrame->inputFrame->secondaryID];

		// Calculate epipolar lines (so we can use them for centroid selection)
		std::vector<cv::Point3f> epipolarLines;
		cv::computeCorrespondEpilines(leftCamera->centroids
			, 1
			, parameters.fundamental_matrix
			, epipolarLines);

		// Sort centroids in left by angle they appear on in right relative to left camera
		std::multimap<float, size_t> leftCameraCentroids_IndexByAngle;
		{
			for (size_t i = 0; i < leftCamera->centroids.size(); i++) {
				auto angle = atan(epipolarLines[i].x / epipolarLines[i].y);
				leftCameraCentroids_IndexByAngle.emplace(angle, i);
			}
		}

		auto angleThreshold = parameters.angleThreshold.getValue() / 360.0f * acos(0) * 4.0f;
		const auto& epipolarDistanceThreshold = parameters.epipolarLineThreshold.getValue();
		const auto& massRatioThreshold = parameters.massRatioThreshold.getValue();

		// Iterate through centroids in right camera to find matches
		std::vector<size_t> matchedCentroidsIndexLeft;
		std::vector<size_t> matchedCentroidsIndexRight;
		std::vector<cv::Point2f> matchedCentroidsLeft;
		std::vector<cv::Point2f> matchedCentroidsRight;
		for (size_t rightCentroidIndex = 0; rightCentroidIndex < rightCamera->centroids.size(); rightCentroidIndex++) {
			auto positionRelativeToLeftCamera = rightCamera->centroids[rightCentroidIndex] - parameters.cameraLeftInCameraRight;
			auto rightCentroidAngle = atan(positionRelativeToLeftCamera.x / positionRelativeToLeftCamera.y);

			for (auto leftCentroidIt = leftCameraCentroids_IndexByAngle.begin()
				; leftCentroidIt != leftCameraCentroids_IndexByAngle.end()
				; leftCentroidIt++) {

				// test the angle threshold
				auto leftAngle = leftCentroidIt->first;
				leftAngle += acos(0); // spin by 180 degrees
				if (leftAngle < rightCentroidAngle - angleThreshold) {
					// too low - continue
					continue;
				}
				else if (leftAngle > rightCentroidAngle + angleThreshold) {
					// too high - break out early
					break;
				}
				else {
					// perform epipolar constraint test
					const auto& leftCentroidIndex = leftCentroidIt->second;
					const auto& rightCentroid = rightCamera->centroids[rightCentroidIndex];

					auto distance = epipolarLines[leftCentroidIndex].dot(cv::Point3f(rightCentroid.x, rightCentroid.y, 1.0f));
					if (abs(distance) <= epipolarDistanceThreshold) {
						// perform mass ratio test
						auto massLeft = leftCamera->moments[leftCentroidIndex].m00;
						auto massRight = rightCamera->moments[rightCentroidIndex].m00;
						auto massRatio = massLeft > massRight
							? massLeft / massRight
							: massRight / massLeft;

						if (massRatio <= massRatioThreshold) {
							// Passed all 2D tests. Let's triangulate
							matchedCentroidsIndexLeft.push_back(leftCentroidIndex);
							matchedCentroidsIndexRight.push_back(rightCentroidIndex);
							matchedCentroidsLeft.push_back(leftCamera->centroids[leftCentroidIndex]);
							matchedCentroidsRight.push_back(rightCentroid);
						}
					}
				}
			}
		}

		// Exit early if empty
		if (matchedCentroidsLeft.empty()) {
			return;
		}


		// Insert test data (warning - we write directly to the output - double check this code if you move too much around)
		if (parameters.includeTestData.getValue()) {
			auto testDataSize = testPointsLeft.size();
			outputFrame->cameraLeftRays = parameters.cameraLeft.unprojectImagePoints(testPointsLeft);
			outputFrame->cameraRightRays = parameters.cameraRight.unprojectImagePoints(testPointsRight);
			
			for (size_t i = 0; i < testDataSize; i++) {
				auto intersection = outputFrame->cameraLeftRays[i].intersect(outputFrame->cameraRightRays[i]);
				outputFrame->worldPoints.push_back(intersection.getMiddle());
				outputFrame->intersections.push_back(std::move(intersection));
			}

			outputFrame->cameraLeftCentroids = testPointsLeft;
			outputFrame->cameraRightCentroids = testPointsRight;
		}

		// Unproject matches and intersect
		auto cameraLeftRays = parameters.cameraLeft.unprojectImagePoints(matchedCentroidsLeft);
		auto cameraRightRays = parameters.cameraRight.unprojectImagePoints(matchedCentroidsRight);
		std::vector<Math::Ray> intersections;
		for (size_t i = 0; i < cameraLeftRays.size(); i++) {
			intersections.push_back(cameraLeftRays[i].intersect(cameraRightRays[i]));
		}

		// Filter matches
		{
			// Check that the intersections are within a maximum length
			auto intersectDistanceThreshold = parameters.intersectDistanceThreshold.getValue();
			for (size_t i = 0; i < cameraLeftRays.size(); i++) {
				if (intersections[i].getLength() <= intersectDistanceThreshold) {
					outputFrame->cameraLeftRays.push_back(cameraLeftRays[i]);
					outputFrame->cameraRightRays.push_back(cameraRightRays[i]);
					outputFrame->intersections.push_back(intersections[i]);
					outputFrame->cameraLeftCentroids.push_back(matchedCentroidsLeft[i]);
					outputFrame->cameraRightCentroids.push_back(matchedCentroidsRight[i]);
					outputFrame->worldPoints.push_back(intersections[i].getMiddle());
				}
			}
		}
	}
}