#include "pch_OP_Triangulate.h"
#include "Process_Triangulate.h"

namespace TD_MoCap {
#pragma mark TriangulateParameters
	//----------
	void
		TriangulateParameters::update()
	{
		{
			auto path = this->leftCameraCalibration.getValue();
			if (!path.empty()) {
				this->cameraLeft.load(path);
			}
		}

		{
			auto path = this->rightCameraCalibration.getValue();
			if (!path.empty()) {
				this->cameraRight.load(path);
			}
		}

		{
			auto path = this->stereoCameraCalibration.getValue();
			if (!path.empty()) {
				cv::FileStorage fs(this->stereoCameraCalibration.getValue(), cv::FileStorage::READ);
				cv::Mat rotationMatrix, translation;
				fs["r"] >> rotationMatrix;
				fs["t"] >> translation;
				this->cameraRight.setExtrinsics(rotationMatrix, translation);
			}
		}

		{
			auto path = this->pointsLeft.getValue();
			if (!path.empty()) {
				cv::FileStorage fs(this->pointsLeft.getValue(), cv::FileStorage::READ);
				fs["Points"] >> this->testData.pointsLeft;
			}
		}

		{
			auto path = this->pointsRight.getValue();
			if (!path.empty()) {
				cv::FileStorage fs(this->pointsLeft.getValue(), cv::FileStorage::READ);
				fs["Points"] >> this->testData.pointsRight;
			}
		}
	}

#pragma mark Process_Triangulate
	//----------
	void
		Process_Triangulate::process(std::shared_ptr<Frames::CentroidsFrame> inputFrame
			,std::shared_ptr<Frames::TriangulateFrame> outputFrame
			, TriangulateParameters& parameters)
	{
		// run test
		{
			const auto& pointsLeft = parameters.testData.pointsLeft;
			const auto& pointsRight = parameters.testData.pointsRight;

			if (!pointsLeft.empty()
				&& !pointsRight.empty())
			{
				if (pointsLeft.size() != pointsRight.size()) {
					throw(Exception("Size mismatch between test points on left and right cameras"));
				}

				// unproject rays
				outputFrame->testData.raysLeft = parameters.cameraLeft.unprojectImagePoints(pointsLeft);
				outputFrame->testData.raysRight = parameters.cameraRight.unprojectImagePoints(pointsRight);
				
				// interesect rays
				auto size = pointsLeft.size();
				outputFrame->testData.intersections.reserve(size);
				for (size_t i = 0; i < size; i++) {
					const auto& leftRay = outputFrame->testData.raysLeft[i];
					const auto& rightRay = outputFrame->testData.raysRight[i];

					outputFrame->testData.intersections.emplace_back(leftRay.intersect(rightRay));
				}
			}
		}
	}
}