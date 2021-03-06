#pragma once

#include "BaseFrame.h"
#include "CameraFrame.h"
#include "pch_MoCapLib.h"
#include "Links/Output.h"
#include "Utils/OpticalFlow.h"
#include "../Constants.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API SynchronisedFrame : public BaseFrame
		{
		public:
			// Make from incoming camera frames
			static std::shared_ptr<SynchronisedFrame> make(const std::map<Links::Output::ID, std::shared_ptr<CameraFrame>>&
				, Links::Output::ID leaderID);

			// Make from serialised data
			static std::shared_ptr<SynchronisedFrame> make(const nlohmann::json& json, const std::filesystem::path& workingFolder);

			std::string getTypeName() const override;

			bool getPreviewImage(cv::Mat&) const override;
			bool getPreviewDAT(Utils::Table&) const override;

			uint64_t getFrameIndex() const override;

			void serialise(nlohmann::json& json, const Utils::Serialisable::Args&) const override;
			void deserialise(const nlohmann::json& json, const std::filesystem::path& workingFolder) override;

			void compute();

			std::map<Links::Output::ID, std::shared_ptr<CameraFrame>> cameraFrames;
			Links::Output::ID leaderID;
			Links::Output::ID secondaryID;

#ifdef MOCAP_ENABLE_OPTICAL_FLOW_ASYNC
			struct {
				std::map<Links::Output::ID, std::shared_ptr<Utils::OpticalFlow::FutureResult>> results;
			} opticalFlow;
#endif
		private:
			SynchronisedFrame();
			SynchronisedFrame(const std::map<Links::Output::ID, std::shared_ptr<CameraFrame>>&
				, Links::Output::ID leaderID);
		};
	}
}