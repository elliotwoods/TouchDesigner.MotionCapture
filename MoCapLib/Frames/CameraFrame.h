#pragma once

#include "BaseFrame.h"
#include <opencv2/opencv.hpp>

#include <chrono>

namespace TD_MoCap {
	class CameraThread;

	namespace Frames {
		class TDMOCAP_API CameraFrame : public BaseFrame
		{
		public:
			struct MetaData
			{
				uint64_t frameIndex;
				std::chrono::microseconds timestamp;

				size_t sensorHeight;
				size_t roiY;
				size_t roiHeight;

				uint8_t reserved[128
					- sizeof(uint64_t)
					- sizeof(std::chrono::microseconds)];
			};

			static std::shared_ptr<CameraFrame> make();
			std::string getTypeName() const override;

			bool getPreviewImage(cv::Mat&) const override;
			bool getPreviewDAT(Utils::Table&) const override;

			uint64_t getFrameIndex() const override;

			void serialise(nlohmann::json& json, const Utils::Serialisable::Args&) const override;
			void deserialise(const nlohmann::json& json, const std::filesystem::path& workingFolder) override;

			std::weak_ptr<CameraThread> cameraThread;

			cv::Mat image;

			MetaData metaData;
		private:
			CameraFrame();
		};
	}
}