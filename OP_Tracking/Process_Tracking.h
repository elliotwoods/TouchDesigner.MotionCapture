#pragma once

#include "MoCapLib.h"

namespace TD_MoCap
{
	struct TrackingParameters : Utils::Parameters {
		Utils::NumberParameter<int> opticalFlowRadius{
			"Optical Flow radius", "px"
			, 10, 10
			, 0, 1024
			, 0, 64
		};

		Utils::NumberParameter<int> searchRadius{
			"Search radius", "px"
			, 5, 5
			, 0, 1024
			, 0, 10
		};

		Utils::ParameterList list{
			&opticalFlowRadius
			, &searchRadius
		};

		void update() override;
	};

	class Process_Tracking
		: public IProcessor<Frames::TriangulateFrame, Frames::TrackingFrame, TrackingParameters>
	{
	public:
		void process(std::shared_ptr<Frames::TriangulateFrame>, std::shared_ptr<Frames::TrackingFrame>, TrackingParameters&) override;
	protected:
		std::shared_ptr<Frames::TrackingFrame> previousFrame;
	};
}