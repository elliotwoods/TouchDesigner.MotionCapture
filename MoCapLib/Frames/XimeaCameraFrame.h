#pragma once

#include "BaseFrame.h"
#include <opencv2/opencv.hpp>

#include <chrono>

namespace TD_MoCap {
	class CameraThread;

	namespace Frames {
		class TDMOCAP_API XimeaCameraFrame : public BaseFrame
		{
		public:
			struct MetaData
			{
				uint64_t frameIndex;
				std::chrono::microseconds timestamp;

				uint8_t reserved[128
					- sizeof(uint64_t)
					- sizeof(std::chrono::microseconds)];
			};

			XimeaCameraFrame();
			std::string getTypeName() const override;

			bool getPreviewImage(cv::Mat&) const override;
			bool getPreviewDAT(Utils::Table&) const override;

			std::weak_ptr<CameraThread> cameraThread;

			cv::Mat image;

			MetaData metaData;
		};
	}
}