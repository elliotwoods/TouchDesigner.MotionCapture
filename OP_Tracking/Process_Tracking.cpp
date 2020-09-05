#include "pch_OP_Tracking.h"
#include "Process_Tracking.h"

#include <opencv2/cudaoptflow.hpp>

// The quad tree is defined with interleaved bits y,x,y,x,y,x (LSB is x)
// supports up to 2048px (11bit) resolution width by chopping off last 3 bits to fit in 8bit
#define IMAGEPOINT_DECIMATION 3

QTPoint imagePointToQTPoint(const cv::Point2f& imagePoint)
{
	QTPoint imagePointX = QTPoint(imagePoint.x) >> IMAGEPOINT_DECIMATION;
	QTPoint imagePointY = QTPoint(imagePoint.y) >> IMAGEPOINT_DECIMATION;

	const auto bits = (sizeof(QTPoint) * 8);
	const auto halfBits = bits / 2;

	QTPoint result;
	for (uint16_t i = 0; i < halfBits; i++) {
		result |= ((imagePointX >> i) & 1) << ((i * 2) + 0);
		result |= ((imagePointY >> i) & 1) << ((i * 2) + 1);
	}

	return result;
}

std::multimap<QTPoint, size_t> imagePointsToQT(const std::vector<cv::Point2f>& imagePoints)
{
	std::multimap<uint16_t, size_t> output;
	for (size_t i=0; i<imagePoints.size(); i++)
	{
		const auto& imagePoint = imagePoints[i];
		output.emplace(imagePointToQTPoint(imagePoint), i);
	}
	return output;
}

std::vector<size_t> getQTPoints(const std::multimap<QTPoint, size_t>& quadTree
	, const QTPoint& point)
{
	std::vector<size_t> result;
	auto findPoint = quadTree.find(point);
	if (findPoint != quadTree.end()) {
		while (findPoint->first == point) {
			result.push_back(findPoint->second);
			findPoint++;
		}
	}
	return result;
}

std::vector<size_t> findAdjacentQTPoints(const std::multimap<QTPoint, size_t>& quadTree
	, const cv::Point2f& imagePoint
	, const uint16_t& maxDistance)
{
	std::vector<size_t> results;

	auto originQTPoint = imagePointToQTPoint(imagePoint);
	auto topLeftQTPoint = imagePointToQTPoint(imagePoint - cv::Point2f(maxDistance, maxDistance));
	auto bottomRightQTPoint = imagePointToQTPoint(imagePoint + cv::Point2f(maxDistance, maxDistance));

	auto findPoints = quadTree.lower_bound(topLeftQTPoint);
	for (
		; findPoints != quadTree.end() && findPoints->first < bottomRightQTPoint
		; findPoints++)
	{
		results.push_back(findPoints->second);
	}
	return results;
}

namespace TD_MoCap {
#pragma mark TrackingParameters
	//----------
	void
		TrackingParameters::update()
	{

	}

#pragma mark Process_Tracking
	//----------
	void
		Process_Tracking::process(std::shared_ptr<Frames::TriangulateFrame> inputFrame
			, std::shared_ptr<Frames::TrackingFrame> outputFrame
			, TrackingParameters& parameters)
	{
		// if we have a previous frame
		if (this->previousFrame) {
			// if the frames are sequential (e.g. no dropped frame)
			if (inputFrame->getFrameIndex() == this->previousFrame->getFrameIndex() + 1) {

				auto leftID = inputFrame->inputFrame->inputFrame->leaderID;
				auto rightID = inputFrame->inputFrame->inputFrame->secondaryID;

				const auto& currentCentroidsLeft = inputFrame->cameraLeftCentroids;
				const auto& currentCentroidsRight = inputFrame->cameraRightCentroids;

				//calculate where current frame centroids might be in previous frames
				std::vector<cv::Point2f> backupCentroidsLeft;
				std::vector<cv::Point2f> backupCentroidsRight;

				const auto opticalFlowMethod = parameters.opticalFlowMethod.getSelectedOption();
				if (opticalFlowMethod == "CPU") {
					std::vector<std::function<void()>> actions;
					auto windowSize = parameters.opticalFlowRadius.getValue() * 2 + 1;

					actions.emplace_back([&] {
						const auto& priorImage = this->previousFrame->inputFrame->inputFrame->cameras[leftID]->greyscale;
						const auto& currentImage = inputFrame->inputFrame->cameras[leftID]->greyscale;
						std::vector<uchar> status;
						std::vector<float> error;
						cv::calcOpticalFlowPyrLK(currentImage
							, priorImage
							, currentCentroidsLeft
							, backupCentroidsLeft
							, status
							, error
							, cv::Size(windowSize, windowSize)
						);
						});

					actions.emplace_back([&] {
						const auto& priorImage = this->previousFrame->inputFrame->inputFrame->cameras[rightID]->greyscale;
						const auto& currentImage = inputFrame->inputFrame->cameras[rightID]->greyscale;
						std::vector<uchar> status;
						std::vector<float> error;
						cv::calcOpticalFlowPyrLK(currentImage
							, priorImage
							, currentCentroidsRight
							, backupCentroidsRight
							, status
							, error
							, cv::Size(windowSize, windowSize)
						);
						});

					// perform the search
					Utils::WorkerGroup::X().parallelFor(actions);
				}
				else if (opticalFlowMethod == "CUDA sparse") {
					auto windowSize = parameters.opticalFlowRadius.getValue() * 2 + 1;
					static auto opticalFlow = cv::cuda::SparsePyrLKOpticalFlow::create(cv::Size(windowSize, windowSize));

					cv::cuda::Stream actionQueue;
					cv::cuda::GpuMat backupCentroidsLeftGPU, backupCentroidsRightGPU;
					{
						const auto& priorImage = cv::cuda::GpuMat(this->previousFrame->inputFrame->inputFrame->cameras[leftID]->greyscale);
						const auto& currentImage = cv::cuda::GpuMat(inputFrame->inputFrame->cameras[leftID]->greyscale);
						auto status = cv::cuda::GpuMat();
						auto error = cv::cuda::GpuMat();

						opticalFlow->calc(currentImage
							, priorImage
							, cv::cuda::GpuMat(currentCentroidsLeft)
							, backupCentroidsLeftGPU
							, status
							, error
							, actionQueue
						);
					}

					{
						const auto& priorImage = cv::cuda::GpuMat(this->previousFrame->inputFrame->inputFrame->cameras[rightID]->greyscale);
						const auto& currentImage = cv::cuda::GpuMat(inputFrame->inputFrame->cameras[rightID]->greyscale);
						auto status = cv::cuda::GpuMat();
						auto error = cv::cuda::GpuMat();

						opticalFlow->calc(currentImage
							, priorImage
							, cv::cuda::GpuMat(currentCentroidsRight)
							, backupCentroidsRightGPU
							, status
							, error
							, actionQueue
						);
					}

					backupCentroidsLeftGPU.download(backupCentroidsLeft, actionQueue);
					backupCentroidsRightGPU.download(backupCentroidsRight, actionQueue);
					actionQueue.waitForCompletion();
				}
				else if (opticalFlowMethod == "CUDA dense async") {
#ifdef MOCAP_ENABLE_OPTICAL_FLOW_ASYNC
					auto beginDownloadTime = std::chrono::high_resolution_clock::now();

					// Download the result and wait
					auto& opticalFlow = inputFrame->inputFrame->inputFrame->opticalFlow;

					// wait for results
					for (const auto& it : opticalFlow.results) {
						it.second->lockThreadJoin.lock();
						it.second->thread.join();
						it.second->lockThreadJoin.unlock();
					}

					auto& flowLeft = opticalFlow.results[leftID]->denseFlowCPU;
					auto& flowRight = opticalFlow.results[rightID]->denseFlowCPU;

					auto endDownloadTime = std::chrono::high_resolution_clock::now();
					auto downloadDuration = endDownloadTime - beginDownloadTime;
					std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(downloadDuration).count() << "ms download time" << std::endl;

					auto getValueBilinear = [](const cv::Mat& img, cv::Point2f pt)
					{
						const int x = (int)pt.x;
						const int y = (int)pt.y;

						int x0 = cv::borderInterpolate(x, img.cols, cv::BORDER_REFLECT_101);
						int x1 = cv::borderInterpolate(x + 1, img.cols, cv::BORDER_REFLECT_101);
						int y0 = cv::borderInterpolate(y, img.rows, cv::BORDER_REFLECT_101);
						int y1 = cv::borderInterpolate(y + 1, img.rows, cv::BORDER_REFLECT_101);

						float a = pt.x - (float)x;
						float c = pt.y - (float)y;

						const auto& upLeft = img.at<cv::Vec2f>(y0, x0);
						const auto& upRight = img.at<cv::Vec2f>(y0, x1);
						const auto& downLeft = img.at<cv::Vec2f>(y1, x0);
						const auto& downRight = img.at<cv::Vec2f>(y1, x1);

						return (upLeft * (1.0f - a) + upRight * a) * (1.0f - c)
							+ (downLeft * (1.0f - a) + downRight * a) * c;
					};

					// 'pipet' the results
					{
						// Left
						{
							for (const auto& currentCentroid : currentCentroidsLeft)
							{
								auto flowVector = getValueBilinear(flowLeft, currentCentroid);
								auto backupCentroid = currentCentroid;
								backupCentroid.x += flowVector[0];
								backupCentroid.y += flowVector[1];
								backupCentroidsLeft.push_back(backupCentroid);
							}
						}
						// Right
						{
							for (const auto& currentCentroid : currentCentroidsRight)
							{
								auto flowVector = getValueBilinear(flowRight, currentCentroid);
								auto backupCentroid = currentCentroid;
								backupCentroid.x += flowVector[0];
								backupCentroid.y += flowVector[1];
								backupCentroidsRight.push_back(backupCentroid);
							}
						}
					}
#else
					throw(Exception("Not implemented"));
#endif
				}
				else if (opticalFlowMethod == "CUDA dense sparse return")
				{
#ifdef MOCAP_ENABLE_OPTICAL_FLOW_ASYNC
					for (auto result : inputFrame->inputFrame->inputFrame->opticalFlow.results) {
						std::unique_lock<std::mutex> lock(result.second->lockThreadJoin);
						result.second->thread.join();
					}

					//left
					{
						auto beginDownloadTime = std::chrono::high_resolution_clock::now();
						cv::cuda::Stream stream;

						auto flowGpu = inputFrame->inputFrame->inputFrame->opticalFlow.results[leftID]->denseFlowGpu;
						std::vector<cv::Mat> patches(currentCentroidsLeft.size());

						for (size_t i = 0; i < currentCentroidsLeft.size(); i++) {
							auto& centroid = currentCentroidsLeft[i];
							auto flowGpuSubsection = cv::cuda::GpuMat(flowGpu, cv::Rect(centroid.x, centroid.y, 1, 1));
							flowGpuSubsection.download(patches[i], stream);
						}
						stream.waitForCompletion();

						auto endDownloadTime = std::chrono::high_resolution_clock::now();
						auto downloadDuration = endDownloadTime - beginDownloadTime;
						std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(downloadDuration).count() << "ms download time" << std::endl;
					}
					throw(Exception("Not implemented"));
#else
					throw(Exception("Not implemented"));
#endif
				}
				else if (opticalFlowMethod == "CUDA sparse + linear") {
					throw(Exception("Not implemented"));
				}


				//search for correspondences between cameras in QT Tree
				const auto maxDistance = parameters.searchRadius.getValue();
				if (parameters.useQuadTree.getValue()) {
#ifdef ENABLE_QUAD_TREE
					// sort the previous centroids, we'll search through them when looking through backup centroids
					const auto previousCentroidsLeftQT = imagePointsToQT(previousFrame->inputFrame->cameraLeftCentroids);
					const auto previousCentroidsRightQT = imagePointsToQT(previousFrame->inputFrame->cameraRightCentroids);

					// iterate over back-inferred centroids
					for (size_t i = 0; i < backupCentroidsLeft.size(); i++)
					{
						// find the centroid (and therefore triangulated point) indices which match the backed-up points
						auto findsLeft = findAdjacentQTPoints(previousCentroidsLeftQT
							, backupCentroidsLeft[i]
							, maxDistance);
						auto findsRight = findAdjacentQTPoints(previousCentroidsRightQT
							, backupCentroidsRight[i]
							, maxDistance);

						// look for matching indices
						for (const auto& findLeft : findsLeft) {
							for (const auto& findRight : findsRight) {
								if (findLeft == findRight) {
									// was a match in the previous frame
									auto findPrevious = this->previousFrame->trackedParticles.find(findLeft);
									if (findPrevious != this->previousFrame->trackedParticles.end()) {
										auto continuingParticle = findPrevious->second; // copy the particle
										continuingParticle.lifeTime++;
										continuingParticle.priorTriangulatedParticleIndex = findPrevious->first;
										continuingParticle.priorTriangulatedParticlePosition = inputFrame->worldPoints[i];
										continuingParticle.priorTriangulatedParticlePosition = continuingParticle.triangulatedParticlePosition;

										outputFrame->trackedParticles.emplace(i, continuingParticle);
									}
									else {
										outputFrame->trackedParticles.emplace(i, Frames::TrackingFrame::Particle{
											 findLeft
											 , inputFrame->worldPoints[i]
											 , inputFrame->worldPoints[i]
											 , 1
											});
									}
								}
							}
						}
					}
#else
					throw(Exception("Quad tree is not implemented"));
#endif
				}

				// Search without QT Tree
				else {
					auto findAdjacentPointsInPreviousFrame = [&](const std::vector<cv::Point2f>& previousFrameCentroids
						, const cv::Point2f& backupPoint
						, float maxDistance)
					{
						auto maxDistance2 = maxDistance * maxDistance;

						std::multimap<float, size_t> findsByDistance2;
						auto min_Y_search = backupPoint.y - maxDistance;
						auto max_Y_search = backupPoint.y + maxDistance;

						for (size_t i = 0; i < previousFrameCentroids.size(); i++) {
							const auto& previousCentroid = previousFrameCentroids[i];
							if (previousCentroid.y < min_Y_search) {
								continue; // skip early
							}
							if (previousCentroid.y > max_Y_search) {
								continue; // skip early
							}

							const auto delta = previousCentroid - backupPoint;
							auto distance2 = delta.dot(delta);
							if (distance2 <= maxDistance2) {
								findsByDistance2.emplace(distance2, i);
							}
						}
						return findsByDistance2;
					};

					for (size_t i = 0; i < backupCentroidsLeft.size(); i++)
					{
						auto candidatePriorCentroidsLeft = findAdjacentPointsInPreviousFrame(previousFrame->inputFrame->cameraLeftCentroids
							, backupCentroidsLeft[i]
							, maxDistance);
						auto candidatePriorCentroidsRight = findAdjacentPointsInPreviousFrame(previousFrame->inputFrame->cameraRightCentroids
							, backupCentroidsRight[i]
							, maxDistance);

						// look for matching indices
						for (const auto& candidatePriorCentroidIndexLeft : candidatePriorCentroidsLeft) {
							for (const auto& candidatePriorCentroidIndexRight : candidatePriorCentroidsRight) {
								if (candidatePriorCentroidIndexLeft.second == candidatePriorCentroidIndexRight.second) {
									// The indices of the centroids match in the Triangulate frame - i.e. these are matched centroids

									const auto & particlePosition = inputFrame->worldPoints[i];

									// Look for prior centroid index in prior tracked particles
									const auto& priorTrackedParticles = this->previousFrame->trackedParticles;
									auto findPriorTrackedParticle = priorTrackedParticles.find(candidatePriorCentroidIndexLeft.second);
									if (findPriorTrackedParticle != priorTrackedParticles.end()) {
										// was a match in the previous frame
										auto continuingParticle = findPriorTrackedParticle->second; // copy the particle
										continuingParticle.lifeTime++;
										continuingParticle.priorTriangulatedParticleIndex = findPriorTrackedParticle->first;
										continuingParticle.priorTriangulatedParticlePosition = continuingParticle.triangulatedParticlePosition;
										continuingParticle.triangulatedParticlePosition = particlePosition;
										outputFrame->trackedParticles.emplace(i, continuingParticle);
									}
									else {
										// this particle isn't being tracked

										// new particle
										outputFrame->trackedParticles.emplace(i, Frames::TrackingFrame::Particle{
											candidatePriorCentroidIndexLeft.second
											, particlePosition
											, particlePosition
											, 1
										});
									}

									break;
								}
							}
						}
					}
				}
			}
		}

		this->previousFrame = outputFrame;
	}
}