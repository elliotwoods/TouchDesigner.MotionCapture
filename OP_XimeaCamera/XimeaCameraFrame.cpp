#include "pch_OP_XimeaCamera.h"
#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	//----------
	XimeaCameraFrame::XimeaCameraFrame()
	{

	}

	//----------
	std::string
		XimeaCameraFrame::getTypeName() const
	{
		return "XimeaCameraFrame";
	}

	//----------
	bool
		XimeaCameraFrame::getPreviewImage(cv::Mat& mat) const {
		mat = this->image; // shallow copy
		return true;
	}

}