#include "pch_MoCapLib.h"
#include "SynchronisedFrame.h"

#include "Utils/WorkerGroup.h"

#include <filesystem>

namespace TD_MoCap {
	namespace Frames {
		//----------
		std::shared_ptr<SynchronisedFrame>
			SynchronisedFrame::make()
		{
			return std::shared_ptr<SynchronisedFrame>(new SynchronisedFrame());
		}

		//----------
		std::shared_ptr<SynchronisedFrame>
			SynchronisedFrame::make(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>& cameraFrames, Links::Output::ID leaderID)
		{
			return std::shared_ptr<SynchronisedFrame>(new SynchronisedFrame(cameraFrames, leaderID));
			
		}

		//----------
		SynchronisedFrame::SynchronisedFrame()
		{
			// used for deserialised frames
			this->leaderID = 0;
		}

		//----------
		SynchronisedFrame::SynchronisedFrame(const std::map<Links::Output::ID, std::shared_ptr<XimeaCameraFrame>>& cameraFrames, Links::Output::ID leaderID)
			: cameraFrames(cameraFrames)
			, leaderID(leaderID)
		{
			this->computeSecondaryID();
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
			auto& cameraFramesJson = json["cameraFrames"];
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
					for (const auto& it : *isComplete) {
						if (!it.second) {
							return;
						}
					}

					// if not, announce to parent
					completeAction();
				};

				cameraFrame.second->serialise(cameraFrameJson["content"], cameraFrameArgs);

				cameraFramesJson.push_back(cameraFrameJson);
			}
			json["leaderID"] = this->leaderID;
		}

		//----------
		void
			SynchronisedFrame::deserialise(const nlohmann::json& json, const std::filesystem::path& workingFolder)
		{
			this->cameraFrames.clear();

			std::mutex lockOutput;
			
			const auto& cameraFramesJson = json["cameraFrames"];

			std::vector<Utils::WorkerThread::Action> actions;

			for (const auto& cameraFrameJson : cameraFramesJson) {
				actions.push_back([this, cameraFrameJson, workingFolder, &lockOutput] {
					auto id = (Links::Output::ID) cameraFrameJson["id"];
					auto cameraFrame = XimeaCameraFrame::make();
					cameraFrame->deserialise(cameraFrameJson["content"], workingFolder);

					{
						std::unique_lock<std::mutex> lock(lockOutput);
						this->cameraFrames.emplace(id, cameraFrame);
					}
				});
			}

			Utils::WorkerGroup::X().parallelFor(actions);

			this->leaderID = json["leaderID"];
			this->computeSecondaryID();
		}

		//----------
		void
			SynchronisedFrame::computeSecondaryID()
		{
			for (auto it : this->cameraFrames) {
				if (it.first != this->leaderID) {
					this->secondaryID = it.first;
					return;
				}
			}
		}
	}
}
