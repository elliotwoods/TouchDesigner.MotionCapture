#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct TriangulateParameters : Utils::Parameters {
		Utils::PathParameter leftCameraCalibration {
			"Left camera calibration"
			, Utils::PathParameter::Type::File
		};

		Utils::PathParameter rightCameraCalibration{
			"Right camera calibration"
			, Utils::PathParameter::Type::File
		};

		Utils::PathParameter stereoCameraCalibration{
			"Stereo camera calibration"
			, Utils::PathParameter::Type::File
		};

		Utils::PathParameter pointsLeft{
			"Points left"
			, Utils::PathParameter::Type::File
		};

		Utils::PathParameter pointsRight{
			"Points right"
			, Utils::PathParameter::Type::File
		};

		Utils::ParameterList list{
			&leftCameraCalibration
			, &rightCameraCalibration
			, &stereoCameraCalibration
			, &pointsLeft
			, &pointsRight
		};

		struct {
			std::vector<cv::Point2f> pointsLeft;
			std::vector<cv::Point2f> pointsRight;
		} testData;

		void update() override;

		Math::Camera cameraLeft, cameraRight;
	};

	class Process_Triangulate
		: public IProcessor<Frames::CentroidsFrame, Frames::TriangulateFrame, TriangulateParameters>
	{
	public:
		void process(std::shared_ptr<Frames::CentroidsFrame>, std::shared_ptr<Frames::TriangulateFrame>, TriangulateParameters&) override;
	};
}