#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct CentroidsParameters : Utils::Parameters {
		Utils::NumberParameter<int> blurRadius{
			"Threshold blur radius", "px"
			, 8, 8
			, 0, 10000
			, 0, 1000
		};

		Utils::NumberParameter<int> threshold{
			"Threshold value", ""
			, 10, 10
			, 0, 255
		};

		Utils::NumberParameter<int> differenceAmplify{
			"Difference amplify", "x"
			, 4, 4
			, 1, 255
			, 1, 16
		};

		Utils::NumberParameter<int> dilationSize{
			"Dilation size", "px"
			, 1, 1
			, 0, 100
			, 0, 32
		};

		Utils::NumberParameter<int> minimumArea{
			"Minimum area", "px (length)"
			, 0, 0
			, 0, 1000
			, 0, 100
		};

		Utils::NumberParameter<int> maximumCount{
			"Maximum count", "centroids"
			, 500, 500
			, 0, 100000
			, 0, 10000
		};

		Utils::ParameterList list{
			&blurRadius
			, &threshold
			, &differenceAmplify
			, &dilationSize
			, &minimumArea
			, &maximumCount
		};
	};

	class Process_Centroids
		: public IProcessor<Frames::SynchronisedFrame, Frames::CentroidsFrame, CentroidsParameters>
	{
	public:
		void process(std::shared_ptr<Frames::SynchronisedFrame>, std::shared_ptr<Frames::CentroidsFrame>, CentroidsParameters&) override;
	};
}