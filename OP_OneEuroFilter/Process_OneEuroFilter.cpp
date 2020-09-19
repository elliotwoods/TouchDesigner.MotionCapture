#include "pch_OP_OneEuroFilter.h"
#include "Process_OneEuroFilter.h"

namespace TD_MoCap {
	//----------
	void
		OneEuroFilterParameters::update()
	{
		this->needsRefreshParameters = true;
	}

	//----------
	void
		Process_OneEuroFilter::process(std::shared_ptr<Frames::FilterAndBinFrame> inputFrame
			, std::shared_ptr<Frames::OneEuroFilterFrame> outputFrame
			, OneEuroFilterParameters& parameters)
	{
		if (!this->previousFrame) {
			// make a blank prior frame if none is available
			this->previousFrame = Frames::OneEuroFilterFrame::make();
		}

		// copy from old into new
		outputFrame->particleBins = this->previousFrame->particleBins;
		if (outputFrame->particleBins.size() != inputFrame->particleBins.size()) {
			outputFrame->particleBins.resize(inputFrame->particleBins.size());
		}

		auto frequency = parameters.frequency.getValue();
		auto minCutOff = parameters.minCutOff.getValue();
		auto dCutOff = parameters.dCutOff.getValue();
		auto beta = parameters.beta.getValue();

		// If we change parameters, re-initialise existing bins
		if (parameters.needsRefreshParameters) {
			for (const auto & particleBin : outputFrame->particleBins) {
				if (particleBin) {
					// Replace the filters and initialise the value
					for (uint8_t i = 0; i < 3; i++) {
						particleBin->filter[i] = Utils::OneEuroFilter(frequency, minCutOff, beta, dCutOff);
						particleBin->filter[i].filter(particleBin->filteredPosition[i]);
					}
				}
			}
			parameters.needsRefreshParameters = false;
		}

		// Add and Update 
		for (size_t i = 0; i < inputFrame->particleBins.size(); i++) {
			const auto & inputBin = inputFrame->particleBins[i];
			auto& outputBin = outputFrame->particleBins[i];

			if (inputBin.occupied) {
				if (inputBin.newBinAssignment || !outputBin) {
					// It's new, or we were empty
					// replace whatever was there before
					outputBin = std::shared_ptr<Frames::OneEuroFilterFrame::ParticleBin>(new Frames::OneEuroFilterFrame::ParticleBin(
						inputBin.UID
						, inputBin.position
						, Utils::OneEuroFilter::Params{ frequency, minCutOff, beta, dCutOff }
						, inputBin.lifetime
					));
				}
				else if(outputBin) {
					// It's an updated bin (and the output is valid), update and filter

					outputBin->UID = inputBin.UID;
					outputBin->rawPosition = inputBin.position;

					for (uint8_t j = 0; j < 3; j++) {
						outputBin->filteredPosition[j] = outputBin->filter[j].filter(inputBin.position[j]);
					}

					outputBin->isNew = false;
					outputBin->lifetime = inputBin.lifetime;
				}
			}
			else {
				outputBin.reset();
			}
		}

		this->previousFrame = outputFrame;
	}
}