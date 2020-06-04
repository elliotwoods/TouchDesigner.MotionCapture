#include "pch_OP_SyncCameras.h"
#include "SynchronisedFrame.h"

#include <filesystem>

namespace TD_MoCap {
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

	//----------
	uint64_t SynchronisedFrame::getFrameIndex() const
	{
		return this->cameraFrames.at(this->leaderID)->metaData.frameIndex;
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