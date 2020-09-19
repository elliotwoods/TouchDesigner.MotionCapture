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
			, 4, 4
			, 0, 1024
			, 0, 10
		};

		Utils::NumberParameter<int> opticalFlowMaxLevel{
			"OF max level", ""
			, 3, 3
			, 1, 8
			, 1, 8
		};

		Utils::NumberParameter<int> opticalFlowMaxIterations{
			"OF max iterations", ""
			, 5, 5
			, 0, 100
			, 0, 100
		};

		Utils::NumberParameter<float> opticalFlowMinDelta{
			"OF min delta", ""
			, .5, 0.5
			, 0.0001, 10
			, 0.0001, 10
		};

#ifdef ENABLE_QUAD_TREE
		Utils::ValueParameter<bool> useQuadTree{
			"Use quad tree"
			, false, false
		};
#endif

		Utils::SelectorParameter opticalFlowMethod{
			"Compute optical flow method"
			, {"CPU", "CUDA sparse", "CUDA dense async", "CUDA dense sparse return", "CUDA sparse + linear" }
			, "CPU"
		};

		Utils::ParameterList list{
			&opticalFlowRadius
			, &searchRadius
			, &opticalFlowMaxLevel
			, &opticalFlowMaxIterations
			, &opticalFlowMinDelta
#ifdef ENABLE_QUAD_TREE
			, &useQuadTree
#endif
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