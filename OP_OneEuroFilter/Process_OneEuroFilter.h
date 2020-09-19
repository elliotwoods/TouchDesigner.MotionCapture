#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct OneEuroFilterParameters : Utils::Parameters {
		Utils::NumberParameter<float> frequency{
			"Frequency", "Hz"
			, 100.0, 100.0f
			, 0.01, 1024
			, 0.01, 1024
		};

		Utils::NumberParameter<float> minCutOff{
			"Min cut off", ""
			, 1.0, 1.0
			, 0.01, 10.0
			, 0.01, 10.0
		};

		Utils::NumberParameter<float> beta{
			"Beta", ""
			, 1.0, 1.0
			, 0.01, 10.0
			, 0.01, 10.0
		};

		Utils::NumberParameter<float> dCutOff{
			"D cut off", ""
			, 1.0, 1.0
			, 0.01, 10.0
			, 0.01, 10.0
		};

		Utils::ParameterList list{
			&frequency
			, &minCutOff
			, &beta
			, &dCutOff
		};

		void update();
		bool needsRefreshParameters = true;
	};

	class Process_OneEuroFilter
		: public IProcessor<Frames::FilterAndBinFrame, Frames::OneEuroFilterFrame, OneEuroFilterParameters>
	{
	public:
		void process(std::shared_ptr<Frames::FilterAndBinFrame>, std::shared_ptr<Frames::OneEuroFilterFrame>, OneEuroFilterParameters&) override;
		std::shared_ptr<Frames::OneEuroFilterFrame> previousFrame;
	};
}