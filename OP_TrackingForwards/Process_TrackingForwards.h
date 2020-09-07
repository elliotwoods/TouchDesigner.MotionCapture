#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct TrackingParameters : Utils::Parameters {
		Utils::NumberParameter<int> opticalFlowRadius{
			"Optical Flow radius", "px"
			, 16, 16
			, 0, 1024
			, 0, 64
		};

		Utils::NumberParameter<int> searchRadius{
			"Search radius", "px"
			, 2, 2
			, 0, 1024
			, 0, 16
		};

		Utils::ParameterList list{
			&opticalFlowRadius
			, &searchRadius
		};
	};

	class Process_TrackingForwards
		: public IProcessor<Frames::TriangulateFrame, Frames::TrackingFrame, TrackingParameters>
	{
	public:
		void process(std::shared_ptr<Frames::TriangulateFrame>, std::shared_ptr<Frames::TrackingFrame>, TrackingParameters&) override;
	protected:
		std::shared_ptr<Frames::TrackingFrame> previousFrame;
	};
}