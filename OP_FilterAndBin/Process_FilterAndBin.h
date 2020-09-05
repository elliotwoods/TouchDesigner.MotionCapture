#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct FilterAndBinParameters : Utils::Parameters {
		Utils::NumberParameter<int> minimumLifetime{
			"Minimum lifetime", "frames"
			, 2, 2
			, 0, 1024
			, 0, 64
		};

		Utils::NumberParameter<int> binCount{
			"Bins", ""
			, 64, 64
			, 0, 1024
			, 0, 256
		};

		Utils::NumberParameter<int> keepAliveFor{
			"Keep alive for", "frames"
			, 3, 3
			, 0, 1024
			, 0, 32
		};

		Utils::NumberParameter<float> resurrectionSearchDistance{
			"Resurrection search distance", "m"
			, 0.005, 0.005
			, 0, 100
			, 0, 0.5
		};

		Utils::ParameterList list{
			&minimumLifetime
			, &binCount
			, &keepAliveFor
			, &resurrectionSearchDistance
		};
	};

	class Process_FilterAndBin
		: public IProcessor<Frames::TrackingFrame, Frames::FilterAndBinFrame, FilterAndBinParameters>
	{
	public:
		void process(std::shared_ptr<Frames::TrackingFrame>, std::shared_ptr<Frames::FilterAndBinFrame>, FilterAndBinParameters&) override;
		std::shared_ptr<Frames::FilterAndBinFrame> previousFrame;

		size_t nextID = 0;
	};
}