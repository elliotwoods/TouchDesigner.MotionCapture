#pragma once
#include "Links/BaseFrame.h"
#include <opencv2/opencv.hpp>

#include <chrono>

namespace TD_MoCap {
	class CameraThread;

	class OP_XIMEACAMERA_API XimeaCameraFrame : public Links::BaseFrame
	{
	public:
		XimeaCameraFrame();
		std::string getTypeName() const override;

		bool getPreviewImage(cv::Mat&) const override;

		std::weak_ptr<CameraThread> cameraThread;

		cv::Mat image;

		struct {
			uint64_t frameIndex;
			std::chrono::microseconds timeStamp;
		} metaData;
	};
}