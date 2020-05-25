#include "pch_SyncCameras.h"
#include "SyncCameras.h"

namespace TD_MoCap {
	//----------
	SyncCameras::SyncCameras(const OP_NodeInfo* info)
	{
	}

	//----------
	SyncCameras::~SyncCameras()
	{

	}

	//----------
	void
		SyncCameras::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		SyncCameras::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		if (!output) {
			return;
		}

		try {
			auto inputCount = inputs->getNumInputs();

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
		SyncCameras::getNumInfoCHOPChans(void* reserved1)
	{
		return 0;
	}

	//----------
	void
		SyncCameras::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved1)
	{
	}

	//----------
	bool
		SyncCameras::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
	{
		return false;
	}

	//----------
	void
		SyncCameras::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1)
	{
	}

	//----------
	void
		SyncCameras::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		
	}

	//----------
	void
		SyncCameras::pulsePressed(const char* name, void* reserved1)
	{

	}

	//----------
	void
		SyncCameras::getErrorString(OP_String* error, void* reserved1)
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
}
