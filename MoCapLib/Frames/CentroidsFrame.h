#pragma once

#include "BaseFrame.h"
#include "XimeaCameraFrame.h"
#include "pch_MoCapLib.h"
#include "Links/Output.h"
#include "SynchronisedFrame.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API CentroidsFrame : public BaseFrame
		{
		public:
			struct Camera {
				cv::Mat greyscale;
				cv::Mat blurred;
				cv::Mat difference;
				cv::Mat binary;

				std::vector<std::vector<cv::Point2i>> contours;
				std::vector<cv::Rect> boundingRects;
				const int dilationSize = 2; // used for calculating moments
				std::vector<cv::Moments> moments;
				std::vector<cv::Point2f> centroids;
			};

			std::string getTypeName() const override;

			bool getPreviewImage(cv::Mat&) const override;
			bool getPreviewDAT(Utils::Table&) const override;
			bool getPreviewCHOP(Utils::ChannelSet&) const override;

			uint64_t getFrameIndex() const override;

			std::map<Links::Output::ID, std::shared_ptr<Camera>> cameras;
			std::shared_ptr<SynchronisedFrame> inputFrame;
		};
	}
}