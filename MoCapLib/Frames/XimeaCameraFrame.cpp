#include "pch_MoCapLib.h"
#include "XimeaCameraFrame.h"
#include "Utils/WorkerGroup.h"

namespace TD_MoCap {
	namespace Frames {
		//----------
		std::shared_ptr<XimeaCameraFrame>
			XimeaCameraFrame::make()
		{
			return std::shared_ptr<XimeaCameraFrame>(new XimeaCameraFrame);
		}

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
			XimeaCameraFrame::getPreviewImage(cv::Mat& mat) const
		{
			mat = this->image; // shallow copy
			return true;
		}

		//----------
		bool
			XimeaCameraFrame::getPreviewDAT(Utils::Table& table) const
		{
			table.newRow() << "frameIndex" << this->metaData.frameIndex;
			table.newRow() << "timestamp" << this->metaData.timestamp.count();
			table.newRow() << "width" << this->image.cols;
			table.newRow() << "height" << this->image.rows;
			table.newRow() << "channels" << this->image.channels();
			return true;
		}

		//----------
		uint64_t
			XimeaCameraFrame::getFrameIndex() const
		{
			return this->metaData.frameIndex;
		}

		//----------
		void
			XimeaCameraFrame::serialise(nlohmann::json& json, const Utils::Serialisable::Args& args) const
		{
			json["frameIndex"] = this->getFrameIndex();
			json["timestamp"] = this->metaData.timestamp.count();
			json["width"] = this->image.cols;
			json["height"] = this->image.rows;
			json["channels"] = this->image.channels();

			auto imageCopy = std::make_shared<cv::Mat>(this->image.clone());
			auto relativeFilePathString = std::to_string(args.frameIndex) + "." + args.imageFormat;
			auto absoluteFilePath = args.folderOut / relativeFilePathString;

			json["filePath"] = relativeFilePathString;

			// Stereo recording test times
			// PNG : 15 fps (858kB)
			// TIF : 20-30 fps (782kB)
			// BMP : 80-116 fps (2.24MB)
			Utils::WorkerGroup::X().perform([imageCopy, args, absoluteFilePath] {
				std::filesystem::create_directories(absoluteFilePath.parent_path());
				cv::imwrite(absoluteFilePath.string(), *imageCopy);
				if (args.onComplete) {
					args.onComplete();
				}
			});
		}

		//----------
		void
			XimeaCameraFrame::deserialise(const nlohmann::json& json, const std::filesystem::path& workingFolder)
		{
			this->metaData.frameIndex = json["frameIndex"];
			this->metaData.timestamp = std::chrono::microseconds((uint64_t) json["timestamp"]);

			auto filename = workingFolder / (std::string) json["filePath"];
			this->image = cv::imread(filename.string(), cv::IMREAD_UNCHANGED);
		}
	}
}