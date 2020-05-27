#include "pch_OP_XimeaCamera.h"
#include "OP_XimeaCamera.h"

namespace TD_MoCap {
	//----------
	OP_XimeaCamera::OP_XimeaCamera(const OP_NodeInfo* info)
	{
		for (const auto parameter : this->parameters.list) {
			parameter->onChange += [this, parameter] {
				this->parameters.stale.insert(parameter);
			};
		}
	}

	//----------
	OP_XimeaCamera::~OP_XimeaCamera()
	{
		this->close();
	}

	//----------
	void
		OP_XimeaCamera::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		OP_XimeaCamera::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
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

			// check all local parameters are up to date
			{
				std::lock_guard<std::mutex> lock(this->parameters.mutex);
				this->parameters.list.updateFromInterface(inputs);
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

					// perform mainloop trigger if we're in that mode
					{
						if (this->cameraThread && this->parameters.trigger.getValue() == "Mainloop") {
							this->cameraThread->requestManualTrigger();
						}
					}

					// update parameter states
					{
						if (this->cameraThread && this->parameters.trigger.getValue() == "Manual") {
							inputs->enablePar("Manualtrigger", true);
						}
						else {
							inputs->enablePar("Manualtrigger", false);
						}
					}

					// get all errors from camera thread
					{
						Exception e;
						while (this->cameraThread->getThreadExceptionQueue().tryReceive(e)) {
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
		OP_XimeaCamera::getNumInfoCHOPChans(void* reserved1)
	{
		return 0;
	}

	//----------
	void
		OP_XimeaCamera::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved1)
	{
	}

	//----------
	bool
		OP_XimeaCamera::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
	{
		return false;
	}

	//----------
	void
		OP_XimeaCamera::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1)
	{
	}

	//----------
	void
		OP_XimeaCamera::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
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
			this->parameters.list.populateInterface(manager);
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
		OP_XimeaCamera::pulsePressed(const char* name, void* reserved1)
	{
		if (strcmp(name, "Reopen") == 0) {
			this->needsReopen = true;
		}
		if (strcmp(name, "Manualtrigger") == 0) {
			if (this->cameraThread) {
				this->cameraThread->requestManualTrigger();
			}
		}
	}

	//----------
	void
		OP_XimeaCamera::getErrorString(OP_String* error, void* reserved1)
	{
		std::lock_guard<std::mutex> lock(this->errorsLock);

		if (!this->errors.empty()) {
			std::string errorString;
			for (const auto& error : this->errors) {
				errorString += error.what() +"\n";
			}
			error->setString(errorString.c_str());
		}
	}

	//----------
	void
		OP_XimeaCamera::open(const OP_Inputs* inputs)
	{
		this->cameraThread = std::make_shared<CameraThread>(this->parameters, this->output);
		this->parameters.stale.clear();
	}

	//----------
	void
		OP_XimeaCamera::close()
	{
		this->cameraThread.reset();
	}
}
