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

		cv::namedWindow("Ximea capture");
		this->requestCapture();
	}

	//----------
	CameraThread::~CameraThread() {
		this->joining = true;
		this->workerThread.performBlocking([this] {
			this->camera.StopAcquisition();
			this->camera.Close();
		});
		cv::destroyAllWindows();
	}

	//----------
	void CameraThread::performInThread(std::function<void(xiAPIplus_Camera&)> action, bool blocking) {
		auto wrappedAction = [this, action] {
			this->runAndFormatExceptions([&] {
				action(this->camera);
			});
		};

		if (blocking) {
			this->workerThread.performBlocking(wrappedAction);
		}
		else {
			this->workerThread.perform(wrappedAction);
		}
	}

	//----------
	void CameraThread::pushToCamera(Utils::AbstractParameter* parameter) {
		auto name = parameter->getName();
		if (name == "Exposure") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<float>*>(parameter);
			if (typedParameter) {
				this->camera.SetExposureTime(typedParameter->getValue() * 1000.0f);
			}
		}
	}

	//----------
	void CameraThread::pullFromCamera(Utils::AbstractParameter* parameter) {
		auto name = parameter->getName();
		if (name == "Exposure") {
			auto typedParameter = dynamic_cast<Utils::ValueParameter<float>*>(parameter);
			if (typedParameter) {
				typedParameter->setValue(this->camera.GetExposureTime() / 1000.0f);
			}
		}
	}

	//----------
	void CameraThread::requestCapture() {
		this->workerThread.perform([this] {
			xiAPIplus_Image image;
			this->camera.GetNextImage(&image);

			// Send the frame
			{
				// Copy pixels into CV format
				auto frame = std::make_shared<XimeaCameraFrame>();
				frame->image = cv::Mat(cv::Size(image.GetWidth(), image.GetHeight())
					, CV_8U
					, image.GetPixels());
				{
					cv::Mat preview;
					cv::pyrDown(frame->image, preview);
					cv::imshow("Ximea capture", preview);
					cv::waitKey(1);
				}

				// Get frame metadata
				{
					auto frameData = image.GetXI_IMG();
					frame->metaData.frameIndex = frameData->acq_nframe;
					frame->metaData.timeStamp = std::chrono::seconds(frameData->tsSec) + std::chrono::microseconds(frameData->tsUSec);
				}

				this->output.send(frame);
			}

			// If we're not closing then request another frame capture
			if (!this->joining) {
				this->requestCapture();
			}
		});
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
			throw(e);
		}
		catch (const cv::Exception& e) {
			throw(Exception(e.what()));
		}
		catch (...) {
			throw(Exception("Unknown exception"));
		}
	}
}