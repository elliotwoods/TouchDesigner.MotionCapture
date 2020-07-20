#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct CentroidsParameters {
		Utils::NumberParameter<int> blurRadius{
			"Threshold blur radius", "px"
			, 100, 100
			, 0, 10000
			, 0, 1000
		};

		Utils::NumberParameter<int> threshold{
			"Threshold value", ""
			, 30, 30
			, 0, 255
			, 0, 255
		};

		Utils::NumberParameter<int> differenceAmplify{
			"Difference amplify", "x"
			, 4, 4
			, 1, 255
			, 1, 16
		};

		Utils::NumberParameter<int> minimumArea{
			"Minimum area", "px (length)"
			, 3, 3
			, 0, 1000
			, 0, 100
		};

		Utils::NumberParameter<int> dilationSize{
			"Dilate bounding rects", "px"
			, 2, 2
			, 0, 256
			, 0, 16
		};

		Utils::ParameterList list{
			&blurRadius
			, &threshold
			, &differenceAmplify
			, &minimumArea
			, &dilationSize
		};
	};

	class Process_Centroids
		: public IProcessor<Frames::SynchronisedFrame, Frames::CentroidsFrame, CentroidsParameters>
	{
	public:
		void process(std::shared_ptr<Frames::SynchronisedFrame>, std::shared_ptr<Frames::CentroidsFrame>, CentroidsParameters&) override;
	};
}