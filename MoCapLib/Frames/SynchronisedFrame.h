#pragma once

#include "BaseFrame.h"
#include "XimeaCameraFrame.h"
#include "pch_MoCapLib.h"
#include "Links/Output.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API SynchronisedFrame : public BaseFrame
		{
		public:
			SynchronisedFrame(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>&
				, Links::Output::ID leaderID);

			struct TDMOCAP_API FileHeader {
				enum {
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

			//void save(const std::filesystem::path&);

			std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>> cameraFrames;
			Links::Output::ID leaderID;
		};
	}
}