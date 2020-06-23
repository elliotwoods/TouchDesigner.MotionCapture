#include "pch_MoCapLib.h"
#include "SynchronisedFrame.h"

#include <filesystem>

namespace TD_MoCap {
	namespace Frames {
		//----------
		SynchronisedFrame::SynchronisedFrame(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>& cameraFrames, Links::Output::ID leaderID)
			: cameraFrames(cameraFrames)
			, leaderID(leaderID)
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

			for (const auto& cameraFrame : this->cameraFrames) {
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

		//----------
		uint64_t
			SynchronisedFrame::getFrameIndex() const
		{
			return this->cameraFrames.at(this->leaderID)->metaData.frameIndex;
		}

		//----------
		void 
			SynchronisedFrame::serialise(nlohmann::json& json, const Utils::Serialisable::Args& args) const
		{
			auto& cameraFrames = json["cameraFrames"];
			auto isComplete = std::make_shared<std::map<Links::Output::ID, bool>>();
			
			for (const auto& cameraFrame : this->cameraFrames) {
				isComplete->emplace(cameraFrame.first, false);
			}

			for (const auto& cameraFrame : this->cameraFrames) {
				nlohmann::json cameraFrameJson;
				cameraFrameJson["id"] = cameraFrame.first;

				// append a subfolder
				auto cameraFrameArgs = args;
				cameraFrameArgs.folderOut /= std::to_string(cameraFrame.first);
				auto cameraFrameIndex = cameraFrame.first;
				auto completeAction = args.onComplete;
				cameraFrameArgs.onComplete = [cameraFrameIndex, isComplete, completeAction] {
					isComplete->at(cameraFrameIndex) = true;

					// check if any are still waiting
					for (const auto& it : * isComplete) {
						if (!it.second) {
							return;
						}
					}

					// if not, announce to parent
					completeAction();
				};

				cameraFrame.second->serialise(cameraFrameJson["content"], cameraFrameArgs);

				cameraFrames.push_back(cameraFrameJson);
			}
		}

		//----------
		void
			SynchronisedFrame::deserialise(const nlohmann::json& json, const Utils::Serialisable::Args& args)
		{

		}

		/*
		//----------
		void
			SynchronisedFrame::save(const std::filesystem::path& folderPath)
		{
			if (this->cameraFrames.size() >= FileHeader::MaxCameraCount) {
				throw(Exception("Too many cameras to save"));
			}

			auto frameIndex = this->getFrameIndex();

			// write data to info file
			{
				// file header
				FileHeader fileHeader;
				{
					fileHeader.cameraCount = this->cameraFrames.size();

					auto metaDataOut = fileHeader.metaData;
					auto cameraIDOut = fileHeader.cameraIDs;
					for (const auto& cameraFrame : this->cameraFrames) {
						*metaDataOut++ = cameraFrame.second->metaData;
						*cameraIDOut++ = cameraFrame.first;
					}
				}

				// file path
				auto filePath = folderPath;
				char fileName[100];
				sprintf_s(fileName, "frame_%010lu", (unsigned long) frameIndex);
				filePath.append(fileName);

				// save the file
				FILE* file;
				auto result = fopen_s(&file, filePath.string().c_str(), "w");
				fwrite(&fileHeader, sizeof(FileHeader), 1, file);
				fclose(file);
			}

			// save each image
			{
				for (const auto& cameraFrame : this->cameraFrames)
				{
					// file path
					auto filePath = folderPath;
					char fileName[100];
					sprintf_s(fileName, "frame_%010lu-%03d.jpg", (unsigned long) frameIndex, (int) cameraFrame.first);
					filePath.append(fileName);

					cv::imwrite(filePath.string()
						, cameraFrame.second->image
						//, {
						//	cv::IMWRITE_PNG_COMPRESSION
						//	, 3
						//}
					);
				}
			}
		}
		*/
	}
}
