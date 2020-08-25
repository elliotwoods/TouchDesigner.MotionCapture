#pragma once

#include <opencv2/cudaoptflow.hpp>
#include "Links/Output.h"

namespace TD_MoCap {
	namespace Utils {
		/// <summary>
		/// Singleton class for async-computing optical flow at Sync/Player OP to be used later in Tracking OP
		/// </summary>
		class OpticalFlow {
		private:
			OpticalFlow();

		public:
			struct FutureResult {
				bool empty = true;
				cv::cuda::GpuMat denseFlow;
				cv::Mat denseFlowCPU;
			};

			static OpticalFlow& X();
			FutureResult calculate(Links::Output::ID cameraIndex, const cv::Mat& image, cv::cuda::Stream& stream);

		private:
			cv::Ptr<cv::cuda::FarnebackOpticalFlow> implementation;

			struct Prior {
				cv::cuda::GpuMat image;
				cv::cuda::GpuMat denseFlow;
			};

			std::map<Links::Output::ID, Prior> priors;

			size_t allocatedWidth = 0;
			size_t allocatedHeight = 0;
		};
	}
}