#include "pch_OP_XimeaCamera.h"
#include "CameraThread.h"
#include "Exception.h"
#include "Frames/CameraFrame.h"

namespace TD_MoCap {
	//----------
	CameraThread::CameraThread(const CameraParameters & cameraParameters, Links::Output& output)
		: output(output)
	{
		this->performInThread([this, &cameraParameters] (xiAPIplus_Camera &) {
			// Open device
			auto serialNumber = cameraParameters.serialNumber.getValue();
			if (!serialNumber.empty()) {
				this->camera.OpenBySN(const_cast<char*>(serialNumber.c_str()));
			}
			else {
				this->camera.OpenFirst();
			}

			// Setup parameters
			this->roiInfo.sensorHeight = this->camera.GetHeight_Maximum();
			for (auto parameter : cameraParameters.list) {
				this->pushToCamera(parameter);
			}

			// Start acquisition
			this->camera.StartAcquisition();

			if (!this->camera.IsAcquisitionActive()) {
				throw(Exception("Failed to open camera"));
			}
		}, true);

		this->requestCapture();
	}

	//----------
	CameraThread::~CameraThread()
	{
		this->joining = true;
		this->workerThread.performBlocking([this] {
			this->camera.StopAcquisition();
			this->camera.Close();
		});
		//		cv::destroyWindow(this->windowName); // this seems to hang
	}

	//----------
	void
		CameraThread::performInThread(std::function<void(xiAPIplus_Camera&)> action, bool blocking)
	{
		if (blocking) {
			this->workerThread.performBlocking([this, action] {
				this->rethrowFormattedExceptions([&] {
					action(this->camera);
				});
			});
		}
		else {
			this->workerThread.perform([this, action] {
				this->rethrowFormattedExceptions([&] {
					action(this->camera);
				});
			});
		}
	}

	//----------
	void
		CameraThread::pushToCamera(Utils::AbstractParameter* parameter)
	{
		auto name = parameter->getName();
		if (name == "Exposure") {
			auto typedParameter = dynamic_cast<Utils::NumberParameter<float>*>(parameter);
			if (typedParameter) {
				this->camera.SetExposureTime(typedParameter->getValue() * 1000.0f);
			}
		}
		else if (name == "Preview") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<bool>*>(parameter);
			if (typedParameter) {
				this->showPreviewWindow = typedParameter->getValue();
				if (!this->showPreviewWindow) {
					cv::destroyWindow(this->windowName);
				}
			}
		}
		else if (name == "Trigger") {
			auto typedParameter = dynamic_cast<Utils::SelectorParameter*> (parameter);
			
			if (typedParameter) {
				bool acquisitionWasActive = camera.IsAcquisitionActive();
				if (acquisitionWasActive) {
					camera.StopAcquisition();
				}

				const auto& trigger = typedParameter->getValue();

				//reference : https://www.ximea.com/support/wiki/apis/XiAPI_Camera_Trigger_and_Synchronization_Signals
				if (trigger == "Freerun") {
					this->camera.SetTriggerSource(XI_TRG_SOURCE::XI_TRG_OFF);
				}
				else if (trigger == "Mainloop") {
					this->camera.SetTriggerSource(XI_TRG_SOURCE::XI_TRG_SOFTWARE);
				}
				else if (trigger == "Manual") {
					this->camera.SetTriggerSource(XI_TRG_SOURCE::XI_TRG_SOFTWARE);
				}
				else if (trigger == "Follower") {
					this->camera.SetGPISelector(XI_GPI_SELECTOR::XI_GPI_PORT1);
					this->camera.SetGPIMode(XI_GPI_TRIGGER);
					this->camera.SetTriggerSource(XI_TRG_SOURCE::XI_TRG_EDGE_RISING);
				}
				else {
					throw(Exception("Trigger mode not supported"));
				}

				// If we're not following, then always enable the trigger output
				if (trigger != "Follower") {
					this->camera.SetGPOSelector(XI_GPO_SELECTOR::XI_GPO_PORT1);
					this->camera.SetGPOMode(XI_GPO_MODE::XI_GPO_EXPOSURE_ACTIVE);
				}

				if (acquisitionWasActive) {
					camera.StartAcquisition();
				}
			}
		}
		else if (name == "Limit bandwidth") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<bool>*>(parameter);
			if (typedParameter) {
				// needs camera to be stopped during change
				auto wasAcquisitionActive = this->camera.IsAcquisitionActive();
				if (wasAcquisitionActive) {
					this->camera.StopAcquisition();
				}
				auto enabled = typedParameter->getValue();
				this->camera.SetBandwidthLimitMode(enabled
					? XI_SWITCH::XI_ON
					: XI_SWITCH::XI_OFF);
				if (wasAcquisitionActive) {
					this->camera.StartAcquisition();
				}
			}
		}
		else if (name == "Target frame rate") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<float>*>(parameter);
			if (typedParameter) {
				this->camera.SetAcquisitionTimingMode(XI_ACQ_TIMING_MODE::XI_ACQ_TIMING_MODE_FRAME_RATE_LIMIT);
				this->camera.SetFrameRate(typedParameter->getValue());
			}
		}
		else if (name == "Maximum bandwidth") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<int>*>(parameter);
			if (typedParameter) {
				// needs camera to be stopped during change
				auto wasAcquisitionActive = this->camera.IsAcquisitionActive();
				if (wasAcquisitionActive) {
					this->camera.StopAcquisition();
				}
				this->camera.SetBandwidthLimit(typedParameter->getValue());
				if (wasAcquisitionActive) {
					this->camera.StartAcquisition();
				}
			}
		}
		else if (name == "ROI portion") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<float>*>(parameter);
			if (typedParameter) {
				// needs camera to be stopped during change
				auto wasAcquisitionActive = this->camera.IsAcquisitionActive();
				if (wasAcquisitionActive) {
					this->camera.StopAcquisition();
				}

				auto roiPortion = typedParameter->getValue();
				roiPortion = MAX(MIN(roiPortion, 1), 0);
				
				int roiHeight = (this->roiInfo.sensorHeight - 1) * roiPortion + 1;
				roiHeight -= roiHeight % this->camera.GetHeight_Increment();

				int roiY = (this->roiInfo.sensorHeight - roiHeight) / 2;
				roiY -= roiY % this->camera.GetOffsetY_Increment();

				auto priorHeight = this->camera.GetHeight();
				auto priorRoiY = this->camera.GetOffsetY();

				// we need to make sure every call is valid (i.e. we don't overstep the image area)
				if (roiHeight > priorHeight) {
					this->camera.SetOffsetY(roiY);
					this->camera.SetHeight(roiHeight);
				}
				else {
					this->camera.SetHeight(roiHeight);
					this->camera.SetOffsetY(roiY);
				}

				this->roiInfo.roiY = roiY;
				this->roiInfo.roiHeight = roiHeight;

				if (wasAcquisitionActive) {
					this->camera.StartAcquisition();
				}
			}
		}
	}

	//----------
	void
		CameraThread::pullFromCamera(Utils::AbstractParameter* parameter)
	{
		auto name = parameter->getName();
		if (name == "Exposure") {
			auto typedParameter = dynamic_cast<Utils::NumberParameter<float>*>(parameter);
			if (typedParameter) {
				typedParameter->setValue(this->camera.GetExposureTime() / 1000.0f);
			}
		}
	}

	//----------
	void
		CameraThread::requestManualTrigger()
	{
		this->performInThread([this] (xiAPIplus_Camera & camera) {
			// it seems this flag will be automatically cleared
			// reference Setup 2 at https://www.ximea.com/support/wiki/apis/XiAPI_Camera_Trigger_and_Synchronization_Signals
			camera.SetTriggerSoftware(1);
		}, false);
	}

	//----------
	xiAPIplus_Camera & 
		CameraThread::getCamera()
	{
		return this->camera;
	}

	//----------
	Utils::WorkerThread &
		CameraThread::getThread()
	{
		return this->workerThread;
	}

	//----------
	Utils::ThreadChannel<Exception>&
		CameraThread::getThreadExceptionQueue()
	{
		return this->workerThread.exceptionsInThread;
	}

	//----------
	void
		CameraThread::requestCapture() {
		this->performInThread([this](xiAPIplus_Camera& camera)
		{
			try {
				// Get any image in the buffer (don't wait)
				auto image = this->camera.GetLastImage();
				auto res = xiGetImage(this->camera.GetCameraHandle()
					, 0
					, image->GetXI_IMG());
				
				if (res == XI_OK) {
					// Copy pixels into CV format
					auto frame = Frames::CameraFrame::make();
					frame->startComputeTimer();

					frame->image = cv::Mat(cv::Size(image->GetWidth(), image->GetHeight())
						, CV_8U
						, image->GetPixels()).clone(); // we need to take a deep copy of the data

					if (this->showPreviewWindow) {
						cv::Mat preview;
						cv::pyrDown(frame->image, preview);
						cv::imshow(this->windowName, preview);
						cv::waitKey(1);
					}

					// Get frame metadata
					{
						auto frameData = image->GetXI_IMG();
						frame->metaData.frameIndex = frameData->acq_nframe;
						frame->metaData.timestamp = std::chrono::seconds(frameData->tsSec) + std::chrono::microseconds(frameData->tsUSec);
						frame->metaData.sensorHeight = this->roiInfo.sensorHeight;
						frame->metaData.roiY = this->roiInfo.roiY;
						frame->metaData.roiHeight = this->roiInfo.roiHeight;
					}

					// don't call shared_from_this on a closing object (we can give up on producing the frame)
					if (this->joining) {
						return;
					}
					frame->cameraThread = this->shared_from_this();

					// Send the frame
					frame->endComputeTimer();
					this->output.send(frame);
				}
				else if (res == XI_TIMEOUT) {
					// ignore - this is fine
				}
				else {
					throw(Exception("Camera capture error"));
				}
				
			}
			catch (...) {
				// request before rethrowing
				if (!this->joining) {
					this->requestCapture();
				}
				throw;
			}

			// If we're not closing then request another frame capture
			if (!this->joining) {
				this->requestCapture();
			}
		}, false);
	}

	//----------
	void
		CameraThread::rethrowFormattedExceptions(std::function<void()> action)
	{
		try {
			action();
		}
		catch (xiAPIplus_Exception e) {
			std::stringstream ss;

			char description[200];
			e.GetDescription(description, sizeof(description));

			ss << "Ximea Error [" << e.GetErrorNumber() << "] " << description;
			throw(Exception(ss.str()));
		}
		catch (...) {
			// remaining exceptions will be handled by the default formatter
			throw;
		}
	}
}