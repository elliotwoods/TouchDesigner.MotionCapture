#pragma once

#include "MoCapLib.h"
#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	class OP_SYNCCAMERAS_API SynchronisedFrame : public Links::BaseFrame
	{
	public:
		SynchronisedFrame(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>&
			, Links::Output::ID leaderID);

		struct OP_XIMEACAMERA_API FileHeader {
			enum{
				MaxCameraCount = 16
			};

			size_t cameraCount;
			XimeaCameraFrame::MetaData metaData[MaxCameraCount];
			Links::Output::ID cameraIDs[MaxCameraCount];
			Links::Output::ID leaderID;
		};

		virtual std::string getTypeName() const override;

		virtual bool getPreviewImage(cv::Mat&) const override;
		virtual bool getPreviewDAT(Utils::Table&) const override;

		uint64_t getFrameIndex() const;

		void save(const std::filesystem::path&);

		std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>> cameraFrames;
		Links::Output::ID leaderID;
	};
}