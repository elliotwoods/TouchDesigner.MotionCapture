#include "pch_MoCapLib.h"
#include "CentroidsFrame.h"

namespace TD_MoCap {
	namespace Frames {
		//----------
		std::shared_ptr<CentroidsFrame::Camera>
			CentroidsFrame::Camera::make()
		{
			return std::shared_ptr<CentroidsFrame::Camera>(new CentroidsFrame::Camera);
		}


		//----------
		std::shared_ptr<CentroidsFrame>
			CentroidsFrame::make()
		{
			return std::shared_ptr<CentroidsFrame>(new CentroidsFrame);
		}

		//----------
		CentroidsFrame::CentroidsFrame()
		{

		}

		//----------
		CentroidsFrame::Camera::Camera()
		{

		}

		//----------
		std::string
			CentroidsFrame::getTypeName() const
		{
			return "CentroidsFrame";
		}

		//----------
		bool
			CentroidsFrame::getPreviewImage(cv::Mat& mat) const
		{
			std::vector<cv::Mat> images;
			images.reserve(this->cameras.size());

			for (const auto& cameraIt : this->cameras) {
				auto & camera = cameraIt.second;
				cv::Mat preview;

				cv::cvtColor(camera->difference, preview, cv::COLOR_GRAY2RGBA);
				
				// draw contours
				cv::drawContours(preview, camera->contours, -1, cv::Scalar(0, 0, 255));


				// draw bounding rectangles
				for (const auto& rect : camera->boundingRects) {
					cv::rectangle(preview, rect, cv::Scalar(255, 0, 0));
				}
				
				images.push_back(preview);
			}

			cv::vconcat(images, mat);

			return true;
		}

		//----------
		bool
			CentroidsFrame::getPreviewDAT(Utils::Table& table) const
		{
			auto contours = std::move(table.newRow() << "Contours");
			auto centroids= std::move(table.newRow() << "Centroids");

			for (const auto& cameraIt : this->cameras) {
				auto& camera = cameraIt.second;
				contours << camera->contours.size();
				centroids << camera->centroids.size();
			}

			return true;
		}

		//----------
		bool
			CentroidsFrame::getPreviewCHOP(Utils::ChannelSet& channels) const
		{
			std::vector<std::string> channelNames;
			for (auto & camera : this->cameras) {
				channelNames.push_back("camera_" + std::to_string(camera.first) + "_x");
				channelNames.push_back("camera_" + std::to_string(camera.first) + "_y");
			}
			channels.setChannels(channelNames);
			
			size_t maxCount = 0;
			for (auto & camera : this->cameras) {
				maxCount = MAX(maxCount, camera.second->centroids.size());
			}
			channels.setSampleCount(maxCount);

			{
				size_t index = 0;
				for (auto& camera : this->cameras) {
					const auto& centroids = camera.second->centroids;
					size_t i = 0;
					for (; i < centroids.size(); i++) {
						channels[index * 2 + 0].samples[i] = centroids[i].x;
						channels[index * 2 + 1].samples[i] = centroids[i].y;
					}
					for (; i < maxCount; i++) {
						channels[index * 2 + 0].samples[i] = 0.0f;
						channels[index * 2 + 1].samples[i] = 0.0f;
					}

					index++;
				}
			}

			return true;
		}

		//----------
		uint64_t
			CentroidsFrame::getFrameIndex() const
		{
			return this->inputFrame->getFrameIndex();
		}
	}
}
