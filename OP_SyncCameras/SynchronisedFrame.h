#pragma once

#include "MoCapLib.h"
#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	class OP_XIMEACAMERA_API SynchronisedFrame : public Links::BaseFrame {
	public:
		SynchronisedFrame(std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>&);
		virtual std::string getTypeName() const override;

		virtual bool getPreviewImage(cv::Mat&) const override;
		virtual bool getPreviewDAT(Utils::Table&) const override;

		std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>> cameraFrames;
	};
}