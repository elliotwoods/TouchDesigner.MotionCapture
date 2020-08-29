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

		Utils::ValueParameter<bool> fullImageOpticalFlow{
			"Full image optical flow"
			, false, false
		};

		Utils::NumberParameter<int> searchRadius{
			"Search radius", "px"
			, 5, 5
			, 0, 1024
			, 0, 10
		};

		Utils::ValueParameter<bool> useQuadTree{
			"Use quad tree"
			, false, false
		};
		
		Utils::SelectorParameter opticalFlowMethod{
			"Compute optical flow method"
			, {"CPU", "CUDA sparse", "CUDA dense async", "CUDA dense sparse return", "CUDA sparse + linear" }
			, "CUDA dense async"
		};

		Utils::ParameterList list{
			&opticalFlowRadius
			, &searchRadius
			, &useQuadTree
			, &opticalFlowMethod
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