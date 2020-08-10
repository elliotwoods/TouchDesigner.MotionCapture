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
		
		outputFrame->particleBins = std::vector<int32_t>(binCount, -1);
		outputFrame->ids = std::vector<size_t>(binCount, -1);
		std::vector<size_t> noPriors;

		// Add all matched particles to the newTracked
		for (const auto& particle : inputFrame->trackedParticles) {
			bool foundPrior = false;
			for (size_t i = 0; i < this->previousFrame->particleBins.size() && i < binCount; i++) {
				if (this->previousFrame->particleBins[i] == particle.second.priorTriangulatedParticleIndex) {
					outputFrame->particleBins[i] = particle.first;
					outputFrame->ids[i] = this->previousFrame->ids[i];
					foundPrior = true;
					break;
				}
			}
			if (!foundPrior) {
				noPriors.push_back(particle.first);
			}
		}

		// Add all new particles (without priors) where a slot exists in the new particle bins
		auto nextNoPrior = noPriors.begin();
		const auto & minLifetime = parameters.minimumLifetime.getValue();
		for (size_t i = 0; i < outputFrame->particleBins.size(); i++) {
			if (nextNoPrior == noPriors.end()) {
				// no more particles to add
				break;
			}
			if (outputFrame->particleBins[i] != -1) {
				// slot is not free
				continue;
			}

			bool isEnded = false;
			while (inputFrame->trackedParticles[*nextNoPrior].lifeTime < minLifetime) {
				nextNoPrior++;
				if (nextNoPrior == noPriors.end()) {
					goto noSlotsRemaining;
				}
			}
			if (isEnded) {
				break;
			}

			outputFrame->particleBins[i] = *nextNoPrior++;
			outputFrame->ids[i] = this->nextID++;
		}

	noSlotsRemaining:

		outputFrame->previousFrame = this->previousFrame;
		this->previousFrame = outputFrame;
	}
}