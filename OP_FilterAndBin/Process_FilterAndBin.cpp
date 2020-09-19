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
		auto frameVelocitySoftUpdate = parameters.frameVelocitySoftUpdate.getValue();

		outputFrame->particleBins = std::vector<Frames::FilterAndBinFrame::ParticleBin>(binCount);
		auto copyBinCount = MIN(previousFrame->particleBins.size(), binCount);

		std::multimap<float, size_t> noPriors;

		// Add all continuing particle bins
		for (const auto& particle : inputFrame->trackedParticles) {
			bool foundPriorBinForThisTrackedParticle = false;
			for (size_t i = 0; i < copyBinCount; i++) {
				const auto& priorBin = this->previousFrame->particleBins[i];

				// If it was alive in the last frame and exists now
				if (priorBin.fullyAlive && priorBin.currentTriangulatedIndex == particle.second.priorTriangulatedIndex) {
					auto& particleBin = outputFrame->particleBins[i];

					particleBin = priorBin;

					particleBin.occupied = true; // redundant
					particleBin.newBinAssignment = false;
					particleBin.currentTriangulatedIndex = particle.first;
					particleBin.position = particle.second.triangulatedParticlePosition;
					particleBin.frameVelocity = particle.second.triangulatedParticlePosition - particle.second.priorTriangulatedParticlePosition;
					particleBin.frameVelocityFiltered = (1.0f - frameVelocitySoftUpdate) * priorBin.frameVelocityFiltered + frameVelocitySoftUpdate * particleBin.frameVelocity;
					particleBin.lifetime = particle.second.lifeTime;

					// These are redundant here
					particleBin.fullyAlive = true;
					particleBin.afterlifeDuration = 0;
					particleBin.resurrectionCount = priorBin.resurrectionCount;

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
			for (size_t i = 0; i < copyBinCount; i++) {
				const auto& priorBin = previousFrame->particleBins[i];
				auto& zombieBin = outputFrame->particleBins[i];

				// Has this bin become empty? i.e. no alive particle was copied above, but last frame was occupied
				// Could be a newly zombie particle, or could be a continuing zombie particle
				if (!zombieBin.occupied && priorBin.occupied) {
					// Has its afterlife expired (afterlifeMaxDuration=0 mean no afterlife)
					if (priorBin.afterlifeDuration < afterlifeMaxDuration) {
						zombieBin = priorBin;

						zombieBin.occupied = true; // redundant
						zombieBin.newBinAssignment = false;
						zombieBin.currentTriangulatedIndex = priorBin.currentTriangulatedIndex;
						zombieBin.position = priorBin.position + priorBin.frameVelocityFiltered;
						zombieBin.frameVelocity = priorBin.frameVelocity;
						zombieBin.frameVelocityFiltered = priorBin.frameVelocityFiltered;
						zombieBin.lifetime = priorBin.lifetime + 1;

						zombieBin.fullyAlive = false;
						zombieBin.afterlifeDuration = priorBin.afterlifeDuration + 1;
						zombieBin.resurrectionCount = priorBin.resurrectionCount;
					}
				}
			}
		}
		
		// Add new particles (and resurrect where available)
		{
			const auto minimumLifetime = parameters.minimumLifetime.getValue();
			auto nextFreeBin = outputFrame->particleBins.begin(); // will be searched each time it's used
			for (const auto& priorLessParticle : noPriors) {
				auto triangulatedIndex = priorLessParticle.second;
				auto& trackedParticle = inputFrame->trackedParticles[triangulatedIndex];

				// Try to resurrect it
				bool isResurrected = false;
				{
					// Search for closest zombie particle bin
					float closestDistance2 = parameters.resurrectionSearchDistance.getValue();
					closestDistance2 *= closestDistance2;
					int foundBinIndex = -1;
					for (size_t binIndex = 0; binIndex < outputFrame->particleBins.size(); binIndex++) {
						const auto& particleBin = outputFrame->particleBins[binIndex];

						if (particleBin.occupied && !particleBin.fullyAlive) {
							// this is a zombie in the current frame - measure its distance
							auto delta = particleBin.position - trackedParticle.triangulatedParticlePosition;
							auto distance2 = glm::dot(delta, delta);
							if (distance2 < closestDistance2) {
								closestDistance2 = distance2;
								foundBinIndex = binIndex;
							}
						}
					}

					if (foundBinIndex != -1) {
						// we found a close-by zombie to use for this particle. The zombie is already inside this bin
						auto& zombieBin = outputFrame->particleBins[foundBinIndex];

						// Some parameters have already been set in the zombieBin update above
						zombieBin.currentTriangulatedIndex = triangulatedIndex;
						zombieBin.position = trackedParticle.triangulatedParticlePosition;
						auto newVelocity = trackedParticle.triangulatedParticlePosition - (zombieBin.position - zombieBin.frameVelocityFiltered);
						zombieBin.frameVelocity = newVelocity;
						zombieBin.lifetime = trackedParticle.lifeTime; // This will result in a downwards jump in lifetime
						zombieBin.frameVelocityFiltered = (1.0f - frameVelocitySoftUpdate) * zombieBin.frameVelocityFiltered + frameVelocitySoftUpdate * newVelocity;

						zombieBin.fullyAlive = true;
						zombieBin.afterlifeDuration = 0;
						zombieBin.resurrectionCount++;

						// don't also make a new bin
						isResurrected = true;
					}
				}


				// Put it in into an empty bin (it's a newly tracked particle without a zombie to resurrect)
				if (!isResurrected) {
					if (trackedParticle.lifeTime > minimumLifetime) {
						// Find the next free bin
						while (nextFreeBin != outputFrame->particleBins.end() && nextFreeBin->occupied) {
							nextFreeBin++;
						}

						if (nextFreeBin != outputFrame->particleBins.end()) {
							nextFreeBin->occupied = true;
							nextFreeBin->newBinAssignment = true;
							nextFreeBin->UID = this->nextUID++;
							nextFreeBin->currentTriangulatedIndex = triangulatedIndex;
							nextFreeBin->position = trackedParticle.triangulatedParticlePosition;
							nextFreeBin->frameVelocity = trackedParticle.triangulatedParticlePosition - trackedParticle.priorTriangulatedParticlePosition;
							nextFreeBin->frameVelocityFiltered = nextFreeBin->frameVelocity;
							nextFreeBin->lifetime = trackedParticle.lifeTime;

							nextFreeBin->fullyAlive = true;
							nextFreeBin->afterlifeDuration = 0;
							nextFreeBin->resurrectionCount = 0;
						}
					}
				}
			}
		}

		this->previousFrame = outputFrame;
	}
}