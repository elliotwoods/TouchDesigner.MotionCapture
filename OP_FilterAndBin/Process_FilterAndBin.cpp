#include "pch_OP_FilterAndBin.h"
#include "Process_FilterAndBin.h"

namespace TD_MoCap {
	//----------
	void
		Process_FilterAndBin::process(std::shared_ptr<Frames::TrackingFrame> inputFrame
			, std::shared_ptr<Frames::FilterAndBinFrame> outputFrame
			, FilterAndBinParameters& parameters)
	{
		if (!this->previousFrame) {
			// make a blank prior frame if none is available
			this->previousFrame = Frames::FilterAndBinFrame::make();
		}

		auto binCount = parameters.binCount.getValue();
		
		outputFrame->particleBins = std::vector<Frames::FilterAndBinFrame::ParticleBin>(binCount);
		
		std::multimap<float, size_t> noPriors;

		// Add all matched particles to the newTracked
		for (const auto& particle : inputFrame->trackedParticles) {
			bool foundPriorBinForThisTrackedParticle = false;
			for (size_t i = 0; i < this->previousFrame->particleBins.size() && i < binCount; i++) {
				const auto& priorBin = this->previousFrame->particleBins[i];
				if (priorBin.fullyAlive && priorBin.currentIndex == particle.second.priorTriangulatedParticleIndex) {
					auto& particleBin = outputFrame->particleBins[i];

					particleBin.occupied = true;
					particleBin.newBinAssignment = false;
					particleBin.currentIndex = particle.first;
					particleBin.position = particle.second.triangulatedParticlePosition;
					particleBin.frameVelocity = particle.second.triangulatedParticlePosition - particle.second.priorTriangulatedParticlePosition;
					particleBin.lifetime = particle.second.lifeTime;

					foundPriorBinForThisTrackedParticle = true;
					break;
				}
			}
			if (!foundPriorBinForThisTrackedParticle) {
				auto leftCentroidMass = inputFrame->inputFrame->cameraLeftMasses[particle.first];
				noPriors.emplace(leftCentroidMass, particle.first);
			}
		}

		// Update afterlife particle bins
		{
			const auto& afterlifeMaxDuration = parameters.keepAliveFor.getValue();
			for (size_t i = 0; i < MIN(previousFrame->particleBins.size(), binCount); i++) {
				const auto& priorBin = previousFrame->particleBins[i];
				auto& currentBin = outputFrame->particleBins[i];
				if (!currentBin.occupied && priorBin.occupied) {
					// absent from current input
					if (priorBin.afterlifeDuration < afterlifeMaxDuration) {
						currentBin.occupied = true;
						currentBin.newBinAssignment = false;
						currentBin.currentIndex = priorBin.currentIndex;
						currentBin.position = priorBin.position + priorBin.frameVelocity;
						currentBin.frameVelocity = priorBin.frameVelocity;
						currentBin.lifetime = priorBin.lifetime + 1;
						currentBin.fullyAlive = false;
						currentBin.afterlifeDuration = priorBin.afterlifeDuration + 1;
					}
				}
			}
		}
		
		// Add new particles (and resurrect where available)
		{
			auto outputBin = outputFrame->particleBins.begin();

			auto nextFreeBin = [&]() {
				while (outputBin != outputFrame->particleBins.end() && outputBin->occupied) {
					outputBin++;
				}
			};

			for (const auto& trackedParticle : inputFrame->trackedParticles) {
				// Try to resurrect it
				{
					float closestDistance2 = parameters.resurrectionSearchDistance.getValue();
					closestDistance2 *= closestDistance2;

					int foundBinIndex = -1;
					for (size_t binIndex = 0; binIndex < outputFrame->particleBins.size(); binIndex++) {
						const auto& particleBin = outputFrame->particleBins[binIndex];

						if (particleBin.occupied && !particleBin.fullyAlive) {
							// this is a zombie
							auto delta = particleBin.position - trackedParticle.second.triangulatedParticlePosition;
							auto distance2 = glm::dot(delta, delta);
							if (distance2 < closestDistance2) {
								closestDistance2 = distance2;
								foundBinIndex = binIndex;
							}
						}
					}

					if (foundBinIndex != -1) {
						// we found a close-by bin to use for this particle
						auto& outputBin = outputFrame->particleBins[foundBinIndex];

						outputBin.occupied = true;
						outputBin.newBinAssignment = false;
						outputBin.currentIndex = trackedParticle.first;
						outputBin.position = trackedParticle.second.triangulatedParticlePosition;
						outputBin.frameVelocity = trackedParticle.second.triangulatedParticlePosition - (outputBin.position - outputBin.frameVelocity);
						outputBin.lifetime = trackedParticle.second.lifeTime;

						outputBin.fullyAlive = true;
						outputBin.afterlifeDuration = 0;
						outputBin.resurrected = true;
					}
				}

				// Put it in into an empty bin
				nextFreeBin();
				if (outputBin != outputFrame->particleBins.end()) {
					outputBin->occupied = true;
					outputBin->newBinAssignment = true;
					outputBin->currentIndex = trackedParticle.first;
					outputBin->position = trackedParticle.second.triangulatedParticlePosition;
					outputBin->frameVelocity = trackedParticle.second.triangulatedParticlePosition - trackedParticle.second.priorTriangulatedParticlePosition;
					outputBin->lifetime = trackedParticle.second.lifeTime;
				}
			}
		}

		this->previousFrame = outputFrame;
	}
}