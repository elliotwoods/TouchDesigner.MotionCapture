#include "pch_MoCapLib.h"
#include "OpticalFlow.h"
#include "Exception.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		OpticalFlow::FutureResult::~FutureResult()
		{
			// if the future result hasn't been consumed by Tracking then we need to join the thread ourselves
			this->lockThreadJoin.lock();
			if (this->thread.joinable() && this->thread.get_id() != std::this_thread::get_id()) {
				this->thread.join();
			}
			this->lockThreadJoin.unlock();
		}

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
		std::shared_ptr<OpticalFlow::FutureResult>
			OpticalFlow::calculate(Links::Output::ID cameraIndex, const cv::Mat& image)
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
				return nullptr;
			}
			auto& prior = findPrior->second;

			// initialise the result
			auto futureResult = std::make_shared<FutureResult>();
			futureResult->thread = std::thread([image, prior, this, cameraIndex, futureResult] () {
				cv::cuda::GpuMat denseFlow;

				cv::cuda::Stream stream;

				// Calculate the optical flow
				this->implementation->calc(cv::cuda::GpuMat(image)
					, prior.image
					, denseFlow
					, stream);

				// Delayed download to CPU
				denseFlow.download(futureResult->denseFlowCPU, stream);

				// store the flow as a starting point for next frame
				this->priors[cameraIndex].denseFlow = denseFlow;
			});

			return futureResult;
		}
	}
}