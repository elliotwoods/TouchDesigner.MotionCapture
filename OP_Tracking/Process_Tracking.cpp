#include "pch_OP_Tracking.h"
#include "Process_Tracking.h"

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
		auto leftID = inputFrame->inputFrame->inputFrame->leaderID;
		auto rightID = inputFrame->inputFrame->inputFrame->secondaryID;

		const auto& currentCentroidsLeft = inputFrame->cameraLeftCentroids;
		const auto & currentCentroidsRight = inputFrame->cameraRightCentroids;
		if (this->previousFrame) {
			//calculate where current frame centroids might be in previous frames
			std::vector<cv::Point2f> backupCentroidsLeft;
			std::vector<cv::Point2f> backupCentroidsRight;
			
			if(!parameters.useQuadTree.getValue()) {
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
			else {
				throw(Exception("We need to build OpenCV with Cuda to get fast optical flow"));
			}

			//search for matches
			const auto maxDistance = parameters.searchRadius.getValue();
			if (parameters.useQuadTree.getValue()) {
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
									outputFrame->trackedParticles.emplace(i, continuingParticle);
								}
								else {
									outputFrame->trackedParticles.emplace(i, Frames::TrackingFrame::Particle{
										 findLeft
										 , 1
										});
								}
							}
						}
					}
				}
			}
			else {
				auto findAdjacentPoints = [&](const std::vector<cv::Point2f>& points
					, const cv::Point2f& point
					, float maxDistance)
				{
					auto maxDistance2 = maxDistance * maxDistance;

					std::vector<size_t> finds;
					for (size_t i = 0; i < points.size(); i++) {
						const auto delta = (points[i] - point);
						if (delta.dot(delta) <= maxDistance2) {
							finds.push_back(i);
						}
					}
					return finds;
				};

				for (size_t i = 0; i < backupCentroidsLeft.size(); i++)
				{
					auto findsLeft = findAdjacentPoints(previousFrame->inputFrame->cameraLeftCentroids
						, backupCentroidsLeft[i]
						, maxDistance);
					auto findsRight = findAdjacentPoints(previousFrame->inputFrame->cameraRightCentroids
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
									outputFrame->trackedParticles.emplace(i, continuingParticle);
								}
								else {
									outputFrame->trackedParticles.emplace(i, Frames::TrackingFrame::Particle{
										 findLeft
										 , 1
										});
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