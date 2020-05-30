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

	//----------
	bool
		XimeaCameraFrame::getPreviewDAT(Utils::Table& table) const {
		table.newRow() << "frameIndex" << this->metaData.frameIndex;
		table.newRow() << "timestamp" << this->metaData.timestamp.count();
		table.newRow() << "width" << this->image.cols;
		table.newRow() << "height" << this->image.rows;
		table.newRow() << "channels" << this->image.channels();
		return true;
	}
}