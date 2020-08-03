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
			static std::shared_ptr<SynchronisedFrame> make(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>&
				, Links::Output::ID leaderID);

			static std::shared_ptr<SynchronisedFrame> make();
			std::string getTypeName() const override;

			bool getPreviewImage(cv::Mat&) const override;
			bool getPreviewDAT(Utils::Table&) const override;

			uint64_t getFrameIndex() const override;

			void serialise(nlohmann::json& json, const Utils::Serialisable::Args&) const override;
			void deserialise(const nlohmann::json& json, const std::filesystem::path& workingFolder) override;

			std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>> cameraFrames;
			Links::Output::ID leaderID;
		private:
			SynchronisedFrame();
			SynchronisedFrame(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>&
				, Links::Output::ID leaderID);
		};
	}
}