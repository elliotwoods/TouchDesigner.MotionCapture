#include "pch_OP_TrackingForwards.h"
#include "Process_TrackingForwards.h"

namespace TD_MoCap {
	//----------
	void
		Process_TrackingForwards::process(std::shared_ptr<Frames::TriangulateFrame> inputFrame
			, std::shared_ptr<Frames::TrackingFrame> outputFrame
			, TrackingParameters& parameters)
	{
		if (this->previousFrame) {
			const auto leftID = inputFrame->inputFrame->inputFrame->leaderID;
			const auto rightID = inputFrame->inputFrame->inputFrame->secondaryID;

			const auto& currentCentroidsLeftRaw = inputFrame->cameraLeftCentroidsRaw;
			const auto& currentCentroidsRightRaw = inputFrame->cameraRightCentroidsRaw;

			const auto priorTrackedSize = this->previousFrame->trackedParticles.size();
			const auto currentTriangulatedSize = currentCentroidsLeftRaw.size();

			const auto searchRadius = parameters.searchRadius.getValue();
			const auto searchRadius2 = searchRadius * searchRadius;
			const auto windowSize = parameters.opticalFlowRadius.getValue() * 2 + 1;

			const auto opticalFlowLevel = 3;
			const auto opticalFlowTermCriteria = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01);

			std::set<size_t> assignedCurrentTriangulatedIndices;
			std::set<size_t> assignedPriorTriangulatedIndices;

			//forwards optical flow on prior tracked particles in left camera
			if(priorTrackedSize > 0) {
				std::vector<size_t> priorTriangulatedIndices(priorTrackedSize);
				std::vector<cv::Point2f> priorCentroidsLeftList(priorTrackedSize);
				std::vector<cv::Point2f> priorCentroidsRightList(priorTrackedSize);
				std::vector<cv::Point2f> lookAheadCentroidsLeft(priorTrackedSize);
				std::vector<cv::Point2f> lookAheadCentroidsRight(priorTrackedSize);

				// velocity update
				{
					auto priorTrackedParticleIt = this->previousFrame->trackedParticles.begin();
					for (size_t i = 0; i < priorTrackedSize; i++, priorTrackedParticleIt++) {

						priorTriangulatedIndices[i] = priorTrackedParticleIt->first;

						{
							const auto& priorCentroid = this->previousFrame->inputFrame->cameraLeftCentroidsRaw[priorTrackedParticleIt->first];
							priorCentroidsLeftList[i] = priorCentroid;
							lookAheadCentroidsLeft[i] = priorCentroid + priorTrackedParticleIt->second.centroidLeftFrameVelocity;
						}

						{
							const auto& priorCentroid = this->previousFrame->inputFrame->cameraRightCentroidsRaw[priorTrackedParticleIt->first];
							priorCentroidsRightList[i] = priorCentroid;
							lookAheadCentroidsRight[i] = priorCentroid + priorTrackedParticleIt->second.centroidRightFrameVelocity;
						}
					}
				}

				// Calculate optical flow
				{
					std::vector<std::function<void()>> actions;

					actions.emplace_back([&] {
						const auto& priorImage = this->previousFrame->inputFrame->inputFrame->cameras[leftID]->difference;
						const auto& currentImage = inputFrame->inputFrame->cameras[leftID]->difference;
						std::vector<uchar> status;
						std::vector<float> error;
						cv::calcOpticalFlowPyrLK(priorImage
							, currentImage
							, priorCentroidsLeftList
							, lookAheadCentroidsLeft
							, status
							, error
							, cv::Size(windowSize, windowSize)
							, opticalFlowLevel
							, opticalFlowTermCriteria
							, cv::OPTFLOW_USE_INITIAL_FLOW
						);

						/*
						{
							cv::Mat preview;
							cv::cvtColor(priorImage, preview, cv::COLOR_GRAY2BGR);
							cv::circle(preview, priorCentroidsLeftList[0], 5, cv::Scalar(255, 0, 0), 1);
							cv::imshow("prior", preview);
						}

						{
							cv::Mat preview;
							cv::cvtColor(currentImage, preview, cv::COLOR_GRAY2BGR);
							cv::circle(preview, lookAheadCentroidsLeft[0], 5, cv::Scalar(255, 0, 0), 1);
							cv::imshow("current", preview);
						}
						cv::waitKey(0);

						std::cout << "pause" << std::endl;
						*/
						});

					actions.emplace_back([&] {
						const auto& priorImage = this->previousFrame->inputFrame->inputFrame->cameras[rightID]->difference;
						const auto& currentImage = inputFrame->inputFrame->cameras[rightID]->difference;
						std::vector<uchar> status;
						std::vector<float> error;
						cv::calcOpticalFlowPyrLK(priorImage
							, currentImage
							, priorCentroidsRightList
							, lookAheadCentroidsRight
							, status
							, error
							, cv::Size(windowSize, windowSize)
							, opticalFlowLevel
							, opticalFlowTermCriteria
							, cv::OPTFLOW_USE_INITIAL_FLOW
						);
						std::cout << "pause" << std::endl;

						});

					Utils::WorkerGroup::X().parallelFor(actions);
				}

				// Search for matching triangulated particles from previous frame
				{
					for (size_t priorTrackedIndex = 0; priorTrackedIndex < priorTrackedSize; priorTrackedIndex++) {
						const auto& lookAheadCentroidLeft = lookAheadCentroidsLeft[priorTrackedIndex];
						const auto& lookAheadCentroidRight = lookAheadCentroidsRight[priorTrackedIndex];

						auto minYLeft = lookAheadCentroidLeft.y - searchRadius;
						auto maxYLeft = lookAheadCentroidLeft.y + searchRadius;

						std::multimap<float, size_t> matches;

						// We're looking for triangulated particles where both the left and right centroid match the lookahead
						for (size_t currentTriangulatedIndex = 0; currentTriangulatedIndex < currentTriangulatedSize; currentTriangulatedIndex++) {
							if (assignedCurrentTriangulatedIndices.find(currentTriangulatedIndex) != assignedCurrentTriangulatedIndices.end()) {
								// skip this one if we're already using it
								continue;
							}

							const auto& currentCentroidLeft = currentCentroidsLeftRaw[currentTriangulatedIndex];
							if (currentCentroidLeft.y < minYLeft) {
								// no more left in this loop which will match (they're in reverse order)
								break;
							}
							if (currentCentroidLeft.y > maxYLeft) {
								continue;
							}
							const auto& currentCentroidRight = currentCentroidsRightRaw[currentTriangulatedIndex];

							float errorSum = 0.0f;

							// check left delta
							{
								const auto delta = lookAheadCentroidLeft - currentCentroidLeft;
								const auto error2 = delta.x * delta.x + delta.y * delta.y;
								if (error2 > searchRadius2) {
									continue;
								}
								errorSum += error2;
							}

							// check right delta
							{
								const auto delta = lookAheadCentroidRight - currentCentroidRight;
								const auto error2 = delta.x * delta.x + delta.y * delta.y;
								if (error2 > searchRadius2) {
									continue;
								}
								errorSum += error2;
							}

							// we have a match for this tracked particle!
							matches.emplace(errorSum, currentTriangulatedIndex);
						}

						if (!matches.empty()) {
							// We have a match!
							auto match = matches.begin();
							const auto currentTriangulatedIndex = match->second;
							const auto priorTriangulatedIndex = priorTriangulatedIndices[priorTrackedIndex];
							outputFrame->trackedParticles.emplace(currentTriangulatedIndex, Frames::TrackingFrame::Particle{
								priorTriangulatedIndex
								, inputFrame->worldPoints[currentTriangulatedIndex]
								, this->previousFrame->inputFrame->worldPoints[priorTriangulatedIndex]
								, currentCentroidsLeftRaw[currentTriangulatedIndex] - priorCentroidsLeftList[priorTrackedIndex]
								, currentCentroidsRightRaw[currentTriangulatedIndex] - priorCentroidsRightList[priorTrackedIndex]
								, this->previousFrame->trackedParticles[priorTriangulatedIndex].lifeTime + 1
								});

							// Mark these triangulated indices as in-use
							assignedCurrentTriangulatedIndices.insert(currentTriangulatedIndex);
							assignedPriorTriangulatedIndices.insert(priorTriangulatedIndex);
						}
					}
				}
			}

			// Search for previously unseen tracked particles
			{
				std::vector<cv::Point2f> currentRemainingCentroidsLeft;
				std::vector<cv::Point2f> currentRemainingCentroidsRight;
				std::vector<size_t> currentRemainingCentroidsTriangulatedIndices;

				// fill the remaining data
				for (size_t i = 0; i < currentTriangulatedSize; i++) {
					if (assignedCurrentTriangulatedIndices.find(i) == assignedCurrentTriangulatedIndices.end()) {
						// not assigned already
						currentRemainingCentroidsLeft.push_back(currentCentroidsLeftRaw[i]);
						currentRemainingCentroidsRight.push_back(currentCentroidsRightRaw[i]);
						currentRemainingCentroidsTriangulatedIndices.push_back(i);
					}
				}

				// Perform backwards optical flow
				std::vector<cv::Point2f> backupCentroidsLeft;
				std::vector<cv::Point2f> backupCentroidsRight;
				{
					std::vector<std::function<void()>> actions;

					actions.emplace_back([&] {
						const auto& priorImage = this->previousFrame->inputFrame->inputFrame->cameras[leftID]->difference;
						const auto& currentImage = inputFrame->inputFrame->cameras[leftID]->difference;
						std::vector<uchar> status;
						std::vector<float> error;
						cv::calcOpticalFlowPyrLK(currentImage
							, priorImage
							, currentRemainingCentroidsLeft
							, backupCentroidsLeft
							, status
							, error
							, cv::Size(windowSize, windowSize)
							, opticalFlowLevel
							, opticalFlowTermCriteria
						);
						});

					actions.emplace_back([&] {
						const auto& priorImage = this->previousFrame->inputFrame->inputFrame->cameras[rightID]->difference;
						const auto& currentImage = inputFrame->inputFrame->cameras[rightID]->difference;
						std::vector<uchar> status;
						std::vector<float> error;
						cv::calcOpticalFlowPyrLK(currentImage
							, priorImage
							, currentRemainingCentroidsRight
							, backupCentroidsRight
							, status
							, error
							, cv::Size(windowSize, windowSize)
							, opticalFlowLevel
							, opticalFlowTermCriteria
						);
						});

					Utils::WorkerGroup::X().parallelFor(actions);
				}

				// Search through backup for particles within search radius
				{
					const auto remainingSize = currentRemainingCentroidsLeft.size();
					const auto priorTriangulatedSize = this->previousFrame->inputFrame->cameraLeftCentroids.size();

					const auto& priorCentroidsLeft = this->previousFrame->inputFrame->cameraLeftCentroids;
					const auto& priorCentroidsRight = this->previousFrame->inputFrame->cameraRightCentroids;

					// NB : we've already filtered out assigned current triangulated before we get to here

					for (size_t backupIterator = 0; backupIterator < remainingSize; backupIterator++) {
						const auto& backupCentroidLeft = backupCentroidsLeft[backupIterator];
						const auto& backupCentroidRight = backupCentroidsRight[backupIterator];
						const auto currentTriangulatedIndex = currentRemainingCentroidsTriangulatedIndices[backupIterator];

						const auto minYLeft = backupCentroidLeft.y - searchRadius;
						const auto maxYLeft = backupCentroidLeft.y + searchRadius;
						std::multimap<float, size_t> matches;
						for (size_t priorTriangulatedIndex = 0; priorTriangulatedIndex < priorTriangulatedSize; priorTriangulatedIndex++) {

							// also check it hasn't already been assigned
							if (assignedPriorTriangulatedIndices.find(priorTriangulatedIndex) != assignedPriorTriangulatedIndices.end()) {
								continue;
							}

							// quick exit logic
							const auto& priorCentroidLeft = priorCentroidsLeft[priorTriangulatedIndex];
							if (priorCentroidLeft.y < minYLeft) {
								// no more left in this loop which will match (they're in reverse order)
								continue;
							}
							if (priorCentroidLeft.y > maxYLeft) {
								continue;
							}
							const auto& priorCentroidRight = priorCentroidsRight[priorTriangulatedIndex];

							float errorSum = 0.0f;

							// check left delta
							{
								const auto delta = priorCentroidLeft - backupCentroidLeft;
								const auto error2 = delta.x * delta.x + delta.y * delta.y;
								if (error2 > searchRadius2) {
									continue;
								}
								errorSum += error2;
							}

							// check right delta
							{
								const auto delta = priorCentroidRight - backupCentroidRight;
								const auto error2 = delta.x * delta.x + delta.y * delta.y;
								if (error2 > searchRadius2) {
									continue;
								}
								errorSum += error2;
							}

							matches.emplace(errorSum, priorTriangulatedIndex);
						}

						if (!matches.empty()) {
							// We have a match!
							auto match = matches.begin();
							const auto priorTriangulatedIndex = match->second;
							outputFrame->trackedParticles.emplace(currentTriangulatedIndex, Frames::TrackingFrame::Particle{
								priorTriangulatedIndex
								, inputFrame->worldPoints[currentTriangulatedIndex]
								, this->previousFrame->inputFrame->worldPoints[priorTriangulatedIndex]
								, currentCentroidsLeftRaw[currentTriangulatedIndex] - priorCentroidsLeft[priorTriangulatedIndex]
								, currentCentroidsRightRaw[currentTriangulatedIndex] - priorCentroidsRight[priorTriangulatedIndex]
								, this->previousFrame->trackedParticles[priorTriangulatedIndex].lifeTime + 1
								});

							// Mark these triangulated indices as in-use
							assignedCurrentTriangulatedIndices.insert(currentTriangulatedIndex); // this isn't used from now on - so this is redundant
							assignedPriorTriangulatedIndices.insert(priorTriangulatedIndex);
						}
					}
				}
			}
		}

		this->previousFrame = outputFrame;
	}
}