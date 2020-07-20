#include "pch_MoCapLib.h"
#include "CentroidsFrame.h"

namespace TD_MoCap {
	namespace Frames {
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

			//cv::imshow("contours", mat);
			//cv::waitKey(1);
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
		uint64_t
			CentroidsFrame::getFrameIndex() const
		{
			return this->inputFrame->getFrameIndex();
		}
	}
}
