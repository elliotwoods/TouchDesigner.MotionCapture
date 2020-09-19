#include "pch_OP_Centroids.h"
#include "Process_Centroids.h"

namespace TD_MoCap {
	//----------
	void
		Process_Centroids::process(std::shared_ptr<Frames::SynchronisedFrame> inputFrame
			,std::shared_ptr<Frames::CentroidsFrame> outputFrame
			, CentroidsParameters& parameters)
	{
		std::vector<Utils::WorkerThread::Action> actions;

		for (auto camera : inputFrame->cameraFrames) {
			auto input = camera.second;
			auto output = Frames::CentroidsFrame::Camera::make();
			outputFrame->cameras[camera.first] = output;

			actions.push_back([this, input, output, &parameters] {
				// convert to greyscale if necessary
				switch (input->image.channels()) {
				case 1:
					output->greyscale = input->image;
					break;
				case 3:
					cv::cvtColor(input->image, output->greyscale, cv::COLOR_RGB2GRAY);
					break;
				case 4:
					cv::cvtColor(input->image, output->greyscale, cv::COLOR_RGBA2GRAY);
					break;
				default:
					throw(Exception("Unsupported number of channels in input image"));
				}

				// local difference filter
				{
					// iterative blur
					{
						auto blurSize = parameters.blurRadius.getValue();
						if (blurSize > 1) {
							cv::blur(output->greyscale
								, output->blurred
								, cv::Size(blurSize / 2, blurSize / 2));

							blurSize /= 2;
							while (blurSize > 1) {
								if (blurSize <= 32) {
									cv::blur(output->blurred
										, output->blurred
										, cv::Size(blurSize, blurSize));
									break;
								}
								cv::blur(output->blurred
									, output->blurred
									, cv::Size(blurSize / 2, blurSize / 2));
								blurSize /= 2;
							}
						}
					}

					// difference
					{
						output->difference = output->greyscale - output->blurred;
						output->difference *= parameters.differenceAmplify.getValue();
					}

					// threshold
					{
						cv::threshold(output->difference
							, output->binary
							, parameters.threshold.getValue()
							, 255
							, cv::THRESH_BINARY);
					}
				}

				// find the contours
				{
					cv::findContours(output->binary
						, output->contours
						, cv::RETR_EXTERNAL
						, cv::CHAIN_APPROX_NONE);
				}

				// bounding rectangles
				const auto dilationSize = parameters.dilationSize.getValue();
				const auto areaThreshold = parameters.minimumArea.getValue() * parameters.minimumArea.getValue();
				{
					for (const auto& contour : output->contours) {
						auto rect = cv::boundingRect(contour);

						//check area
						if (areaThreshold > 0 && rect.area() <= areaThreshold) {
							continue;
						}

						//check if it touches edge of frame (we use a threshold of 2px for rejections)
						{
							const int distanceThreshold = dilationSize;
							auto bottomRight = rect.br();
							if (rect.x <= distanceThreshold
								|| rect.y <= distanceThreshold
								|| output->greyscale.cols - bottomRight.x <= distanceThreshold
								|| output->greyscale.rows - bottomRight.y <= distanceThreshold) {
								continue;
							}
						}

						//create a dilated rect for finding moments
						auto dilatedRect = rect;
						{
							dilatedRect.x -= dilationSize;
							dilatedRect.y -= dilationSize;
							dilatedRect.width += dilationSize * 2;
							dilatedRect.height += dilationSize * 2;
						}
						auto moment = cv::moments(output->greyscale(dilatedRect));

						output->boundingRects.push_back(dilatedRect);
						output->moments.push_back(moment);
					}
				}

				// get moment centers
				output->centroids.reserve(output->boundingRects.size());
				for (size_t i = 0; i < output->boundingRects.size(); i++) {
					auto& moment = output->moments[i];
					auto& bounds = output->boundingRects[i];

					output->centroids.emplace_back(
						moment.m10 / moment.m00 + (float)bounds.x - dilationSize
						, moment.m01 / moment.m00 + (float)bounds.y - dilationSize
					);
				}

				// trim output
				{
					auto maxCentroids = parameters.maximumCount.getValue();
					if (output->centroids.size() > maxCentroids) {
						// store them in order by mass (negative so largest will end up first)
						std::multimap<float, size_t> indexByMass;
						for (size_t i = 0; i < output->centroids.size(); i++) {
							indexByMass.emplace(-output->moments[i].m00, i);
						}

						// store the trimmed list by index so that it will be sorted by the natural contour ordering in the output vector
						std::set<size_t> trimmedIndices;
						for (const auto& it : indexByMass) {
							trimmedIndices.emplace(it.second);
						}

						// trim the output
						{
							std::vector<std::vector<cv::Point2i>> trimmedContours;
							std::vector<cv::Rect> trimmedBoundingRects;
							std::vector<cv::Moments> trimmedMoments;
							std::vector<cv::Point2f> trimmedCentroids;

							trimmedContours.reserve(maxCentroids);
							trimmedBoundingRects.reserve(maxCentroids);
							trimmedMoments.reserve(maxCentroids);
							trimmedCentroids.reserve(maxCentroids);

							for(const auto & index : trimmedIndices) {
								trimmedContours.push_back(std::move(output->contours[index]));
								trimmedBoundingRects.push_back(output->boundingRects[index]);
								trimmedMoments.push_back(output->moments[index]);
								trimmedCentroids.push_back(output->centroids[index]);
							}

							//swap the trimmed values in 
							std::swap(trimmedContours, output->contours);
							std::swap(trimmedBoundingRects, output->boundingRects);
							std::swap(trimmedMoments, output->moments);
							std::swap(trimmedCentroids, output->centroids);
						}
					}
				}
			});
		}

		Utils::WorkerGroup::X().parallelFor(actions);
	}
}