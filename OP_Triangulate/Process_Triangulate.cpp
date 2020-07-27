#include "pch_OP_Triangulate.h"
#include "Process_Triangulate.h"

namespace TD_MoCap {
#pragma mark CameraCalibration
	//----------
	void
		TriangulateParameters::CameraCalibration::load(const std::string& path)
	{
		if (!path.empty()) {
			cv::FileStorage fs(path, cv::FileStorage::READ);
			fs["camera_matrix"] >> this->cameraMatrix;
			fs["dist_coeff"] >> this->distortionCoefficients;
			fs["size"] >> this->size;
		}
	}

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
		
	}
}