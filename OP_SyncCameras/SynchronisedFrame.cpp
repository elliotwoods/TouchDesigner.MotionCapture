#include "pch_OP_SyncCameras.h"
#include "SynchronisedFrame.h"

namespace TD_MoCap {
	//----------
	SynchronisedFrame::SynchronisedFrame(std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>& cameraFrames)
		: cameraFrames(cameraFrames)
	{
	}

	//----------
	std::string
		SynchronisedFrame::getTypeName() const
	{
		return "SyncrhonisedFrame";
	}

	//----------
	bool
		SynchronisedFrame::getPreviewImage(cv::Mat& mat) const
	{
		std::vector<cv::Mat> images;

		for (const auto & cameraFrame : this->cameraFrames) {
			if (cameraFrame.second) {
				images.push_back(cameraFrame.second->image);
			}
		}
		cv::vconcat(images, mat);
		return true;
	}

	//----------
	bool
		SynchronisedFrame::getPreviewDAT(Utils::Table& table) const
	{
		table.newRow() << "ID" << "frameIndex" << "timeStamp";
		for (const auto& cameraFrame : this->cameraFrames) {
			if (cameraFrame.second) {
				table.newRow() << cameraFrame.first << cameraFrame.second->metaData.frameIndex << cameraFrame.second->metaData.timestamp.count();
			}
		}
		return true;
	}
}