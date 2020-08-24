#include "pch_MoCapLib.h"
#include "OpticalFlow.h"
#include "Exception.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		OpticalFlow::OpticalFlow()
		{
			
		}

		//----------
		OpticalFlow & 
			OpticalFlow::X()
		{
			static std::unique_ptr<OpticalFlow> instance(new OpticalFlow());
			return *instance;
		}

		//----------
		OpticalFlow::FutureResult
			OpticalFlow::calculate(Links::Output::ID cameraIndex, const cv::Mat& image, cv::cuda::Stream & stream)
		{
			if (image.empty()) {
				throw(Exception("Cannot calculate optical flow on empty image"));
			}

			if (this->allocatedWidth != image.cols || this->allocatedHeight != image.rows) {
				this->implementation = cv::cuda::FarnebackOpticalFlow::create();
				this->allocatedWidth = image.cols;
				this->allocatedHeight = image.rows;
			}

			// get the prior image/flow from our local storage
			auto findPrior = this->priors.find(cameraIndex);

			// check we have a prior
			if (findPrior == this->priors.end()) {
				// store the image
				this->priors.emplace(cameraIndex, Prior {
					cv::cuda::GpuMat(image)
					, cv::cuda::GpuMat()
					});

				// return an empty result
				FutureResult result;
				result.empty = true;
				return result;
			}
			auto& prior = findPrior->second;

			// initialise the result
			FutureResult futureResult;
			futureResult.empty = false;

			// Calculate the optical flow
			this->implementation->calc(cv::cuda::GpuMat(image)
				, prior.image
				, futureResult.denseFlow
				, stream);
			
			// store the flow as a starting point for next frame
			this->priors[cameraIndex].denseFlow = futureResult.denseFlow;

			return futureResult;
		}
	}
}