#include "pch_OP_XimeaCamera.h"
#include "CameraThread.h"
#include "Exception.h"
#include "XimeaCameraFrame.h"

namespace TD_MoCap {
	//----------
	CameraThread::CameraThread(const OP_Inputs* inputs, const Utils::ParameterList & parameterList, Links::Output& output)
	: output(output) {
		auto serialNumber = std::string(inputs->getParString("Serial"));

		this->workerThread.performBlocking([this, serialNumber, &parameterList] {
			this->runAndFormatExceptions([&] {
				// Open device
				if (!serialNumber.empty()) {
					this->camera.OpenBySN(const_cast<char*>(serialNumber.c_str()));
				}
				else {
					this->camera.OpenFirst();
				}

				// Setup parameters
				for (auto parameter : parameterList) {
					this->pushToCamera(parameter);
				}

				// Start acquisition
				this->camera.StartAcquisition();
			});

			if (!this->camera.IsAcquisitionActive()) {
				throw(Exception("Failed to open camera"));
			}
		});

		this->requestCapture();
	}

	//----------
	CameraThread::~CameraThread() {
		this->joining = true;
		this->workerThread.performBlocking([this] {
			this->camera.StopAcquisition();
			this->camera.Close();
		});
		cv::destroyWindow(this->windowName);
	}

	//----------
	void CameraThread::performInThread(std::function<void(xiAPIplus_Camera&)> action, bool blocking) {
		if (blocking) {
			this->workerThread.performBlocking([this, action] {
				this->runAndFormatExceptions([&] {
					action(this->camera);
				});
			});
		}
		else {
			this->workerThread.perform([this, action] {
				try {
					this->runAndFormatExceptions([&] {
						action(this->camera);
					});
				}
				catch (const Exception& e) {
					this->errorsFromThread.send(e);
				}
			});
		}
	}

	//----------
	void CameraThread::pushToCamera(Utils::AbstractParameter* parameter) {
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
	}

	//----------
	void CameraThread::pullFromCamera(Utils::AbstractParameter* parameter) {
		auto name = parameter->getName();
		if (name == "Exposure") {
			auto typedParameter = dynamic_cast<Utils::NumberParameter<float>*>(parameter);
			if (typedParameter) {
				typedParameter->setValue(this->camera.GetExposureTime() / 1000.0f);
			}
		}
	}

	//----------
	void CameraThread::requestCapture() {
		this->performInThread([this](xiAPIplus_Camera& camera) {
			try {
				xiAPIplus_Image image;
				camera.GetNextImage(&image);

				// Copy pixels into CV format
				auto frame = std::make_shared<XimeaCameraFrame>();
				frame->image = cv::Mat(cv::Size(image.GetWidth(), image.GetHeight())
					, CV_8U
					, image.GetPixels());

				if (this->showPreviewWindow) {
					cv::Mat preview;
					cv::pyrDown(frame->image, preview);
					cv::imshow(this->windowName, preview);
					cv::waitKey(1);
				}

				// Get frame metadata
				{
					auto frameData = image.GetXI_IMG();
					frame->metaData.frameIndex = frameData->acq_nframe;
					frame->metaData.timeStamp = std::chrono::seconds(frameData->tsSec) + std::chrono::microseconds(frameData->tsUSec);
				}

				// Send the frame
				this->output.send(frame);
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
	void CameraThread::runAndFormatExceptions(std::function<void()> action) {
		try {
			action();
		}
		catch (const std::exception& e) {
			throw(Exception(e.what()));
		}
		catch (xiAPIplus_Exception e) {
			std::stringstream ss;

			char description[200];
			e.GetDescription(description, sizeof(description));

			ss << "Ximea Error [" << e.GetErrorNumber() << "] " << description;
			throw(Exception(ss.str()));
		}
		catch (const Exception& e) {
			throw;
		}
		catch (const cv::Exception& e) {
			throw(Exception(e.what()));
		}
		catch (...) {
			throw(Exception("Unknown exception"));
		}
	}
}