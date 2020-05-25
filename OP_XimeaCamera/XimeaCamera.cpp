#include "pch_OP_XimeaCamera.h"
#include "XimeaCamera.h"

namespace TD_MoCap {
	//----------
	XimeaCamera::XimeaCamera(const OP_NodeInfo* info)
	{
	}

	//----------
	XimeaCamera::~XimeaCamera()
	{
		this->close();
	}

	//----------
	void
		XimeaCamera::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		XimeaCamera::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		if (!output) {
			return;
		}

		try {
			// Handle pulse on 'Re-Open'
			if (this->needsReopen) {
				this->close();
				this->needsReopen = false;
			}

			// Handle change in switch
			{
				auto enabled = inputs->getParInt("Enabled") == 1;
				if (enabled) {
					// Camera should be open
					if (!this->cameraThread) {
						// Camera is closed
						this->open(inputs);
					}
				}
				else {
					// Camera should be closed
					if (this->cameraThread) {
						this->close();
					}
				}
			}

			// If enabled...
			{
				auto enabled = (bool)this->cameraThread;
				inputs->enablePar("Reopen", enabled);

				if (enabled) {
					// check all parameters are up to date on device
					{
						std::lock_guard<std::mutex> lock(this->parameters.mutex);

						for (auto parameter : this->parameters.list) {
							// float parameter
							{
								auto typedParameter = dynamic_cast<Utils::NumberParameter<float>*>(parameter);
								if (typedParameter) {
									auto valueInTD = (float)inputs->getParDouble(typedParameter->getName().c_str(), 0);
									auto valueInCameraDriver = typedParameter->getValue();
									if (valueInTD != valueInCameraDriver) {
										typedParameter->setValue(valueInTD);
										{
											this->parameters.stale.insert(parameter);
										}
									}
								}
							}

							// int parameter
							{
								auto typedParameter = dynamic_cast<Utils::NumberParameter<int>*>(parameter);
								if (typedParameter) {
									auto valueInTD = (float)inputs->getParDouble(typedParameter->getName().c_str(), 0);
									auto valueInCameraDriver = typedParameter->getValue();
									if (valueInTD != valueInCameraDriver) {
										typedParameter->setValue(valueInTD);
										{
											this->parameters.stale.insert(parameter);
										}
									}
								}
							}

							// bool parameter
							{
								auto typedParameter = dynamic_cast<Utils::ValueParameter<bool>*>(parameter);
								if (typedParameter) {
									auto valueInTD = (float)inputs->getParInt(typedParameter->getName().c_str(), 0);
									auto valueInCameraDriver = typedParameter->getValue() ? 1 : 0;
									if (valueInTD != valueInCameraDriver) {
										typedParameter->setValue(valueInTD);
										{
											this->parameters.stale.insert(parameter);
										}
									}
								}
							}
						}
					}

					// perform the updates
					this->cameraThread->performInThread([this](xiAPIplus_Camera& camera) {
						try {
							std::lock_guard<std::mutex> lock(this->parameters.mutex);

							for (auto parameter : this->parameters.stale) {
								this->cameraThread->pushToCamera(parameter);
							}
							this->parameters.stale.clear();
						}
						catch (const Exception& e) {
							std::lock_guard<std::mutex> lock(this->errorsLock);
							this->errors.push_back(e);
						}
					}, false);

					// get all errors from camera thread
					{
						Exception e;
						while (this->cameraThread->errorsFromThread.tryReceive(e)) {
							this->errors.push_back(e);
						}
					}
				}
				else {
				
				}
			}

			// Update output
			this->output.update();
			this->output.populateMainThreadOutput(output);
		}
		catch (const Exception & e) {
			std::lock_guard<std::mutex> lock(this->errorsLock);
			this->errors.push_back(e);
		}
	}

	//----------
	int32_t
		XimeaCamera::getNumInfoCHOPChans(void* reserved1)
	{
		return 0;
	}

	//----------
	void
		XimeaCamera::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved1)
	{
	}

	//----------
	bool
		XimeaCamera::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
	{
		return false;
	}

	//----------
	void
		XimeaCamera::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1)
	{
	}

	//----------
	void
		XimeaCamera::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		// Serial number
		{
			OP_StringParameter param;

			param.name = "Serial";
			param.label = "Serial Number";
			param.defaultValue = "";

			auto res = manager->appendString(param);
			assert(res == OP_ParAppendResult::Success);
		}

		// Enabled
		{
			OP_NumericParameter param;

			param.name = "Enabled";
			param.label = param.name;

			param.defaultValues[0] = 0.0;

			auto res = manager->appendToggle(param);
			assert(res == OP_ParAppendResult::Success);
		}

		// Re-Open
		{
			OP_NumericParameter param;

			param.name = "Reopen";
			param.label = "Re-Open";

			auto res = manager->appendPulse(param);
			assert(res == OP_ParAppendResult::Success);
		}

		// camera parameters
		{
			for (const auto & parameter : this->parameters.list) {
				{
					auto floatParameter = dynamic_cast<Utils::NumberParameter<float> *>(parameter);
					if (floatParameter) {
						OP_NumericParameter param;

						param.name = floatParameter->getName().c_str();
						auto label = (floatParameter->getName() + " [" + floatParameter->getUnits() + "]");
						param.label = label.c_str();

						param.defaultValues[0] = floatParameter->getDefaultValue();
						param.minSliders[0] = floatParameter->getSliderMin();
						param.maxSliders[0] = floatParameter->getSliderMax();
						param.minValues[0] = floatParameter->getMin();
						param.maxValues[0] = floatParameter->getMax();

						auto minMaxEnabled = floatParameter->getMax() > floatParameter->getMin();
						param.clampMins[0] = minMaxEnabled;
						param.clampMaxes[0] = minMaxEnabled;

						auto res = manager->appendFloat(param);
						assert(res == OP_ParAppendResult::Success);

						continue;
					}

					auto intParameter = dynamic_cast<Utils::NumberParameter<int>*>(parameter);
					if (intParameter) {
						OP_NumericParameter param;

						param.name = intParameter->getName().c_str();
						auto label = (intParameter->getName() + " [" + intParameter->getUnits() + "]");
						param.label = label.c_str();

						param.defaultValues[0] = intParameter->getDefaultValue();
						param.minSliders[0] = intParameter->getMin();
						param.maxSliders[0] = intParameter->getMax();

						auto minMaxEnabled = intParameter->getMax() > intParameter->getMin();
						param.clampMins[0] = minMaxEnabled;
						param.clampMaxes[0] = minMaxEnabled;

						auto res = manager->appendInt(param);
						assert(res == OP_ParAppendResult::Success);

						continue;
					}

					auto boolParameter = dynamic_cast<Utils::ValueParameter<bool>*>(parameter);
					if (boolParameter) {
						OP_NumericParameter param;

						param.name = boolParameter->getName().c_str();
						param.label = param.name;

						param.defaultValues[0] = boolParameter->getDefaultValue() ? 1 : 0;

						auto res = manager->appendToggle(param);
						assert(res == OP_ParAppendResult::Success);

						continue;
					}
				}
			}
		}

		// Trigger
		{
			OP_StringParameter param;

			param.name = "Trigger";
			param.label = param.name;

			param.defaultValue = "Off";

			const char* options[] = { "Off", "Manual", "Leader", "Follower" };
			auto res = manager->appendMenu(param, 4, options, options);
			assert(res == OP_ParAppendResult::Success);
		}

		// One shot
		{
			OP_NumericParameter param;

			param.name = "Manualtrigger";
			param.label = "Manual Trigger";

			auto res = manager->appendPulse(param);
			assert(res == OP_ParAppendResult::Success);
		}
	}

	//----------
	void
		XimeaCamera::pulsePressed(const char* name, void* reserved1)
	{
		if (!strcmp(name, "Reopen")) {
			this->needsReopen = true;
		}
	}

	//----------
	void
		XimeaCamera::getErrorString(OP_String* error, void* reserved1)
	{
		std::lock_guard<std::mutex> lock(this->errorsLock);

		if (!this->errors.empty()) {
			std::string errorString;
			for (const auto& error : this->errors) {
				errorString += error + "\n";
			}
			error->setString(errorString.c_str());
		}
	}

	//----------
	void
		XimeaCamera::open(const OP_Inputs* inputs)
	{
		this->cameraThread = std::make_shared<CameraThread>(inputs, this->parameters.list, this->output);
	}

	//----------
	void
		XimeaCamera::close()
	{
		this->cameraThread.reset();
	}
}
