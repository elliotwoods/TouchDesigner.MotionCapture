#include "pch_OP_SyncCameras.h"
#include "OP_SyncCameras.h"

namespace TD_MoCap {
	//----------
	OP_SyncCameras::OP_SyncCameras(const OP_NodeInfo* info)
	{
	}

	//----------
	OP_SyncCameras::~OP_SyncCameras()
	{

	}

	//----------
	void
		OP_SyncCameras::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		OP_SyncCameras::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		this->synchroniser.getParameters().updateFromInterface(inputs);

		try {
			// gather new IDs and check our list
			{
				auto inputCount = inputs->getNumInputs();
				std::vector<Links::Output::ID> newOutputIDs;
				for (int i = 0; i < inputCount; i++) {
					Links::Output::ID id;
					if (Links::Output::getIDFromDAT(inputs->getInputDAT(i), id)) {
						newOutputIDs.push_back(id);
					}
				}

				this->synchroniser.checkConnections(newOutputIDs);
			}

			// Update output
			this->synchroniser.output.update();
			this->synchroniser.output.populateMainThreadOutput(output);
		}
		catch (const Exception & e) {
			this->errors.push_back(e);
		}

		//receive all errors from thread
		{
			auto& errorsFromThread = this->synchroniser.getThread().exceptionsInThread;
			Exception e;
			while (errorsFromThread.tryReceive(e)) {
				this->errors.push_back(e);
			}
		}
	}

	//----------
	int32_t
		OP_SyncCameras::getNumInfoCHOPChans(void* reserved1)
	{
		return 0;
	}

	//----------
	void
		OP_SyncCameras::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved1)
	{
	}

	//----------
	bool
		OP_SyncCameras::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
	{
		return false;
	}

	//----------
	void
		OP_SyncCameras::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1)
	{
	}

	//----------
	void
		OP_SyncCameras::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		// resync
		{
			OP_NumericParameter param;

			param.name = "Resync";
			param.label = "Re-Sync";

			auto res = manager->appendPulse(param);
			assert(res == OP_ParAppendResult::Success);
		}

		this->synchroniser.getParameters().populateInterface(manager);
	}

	//----------
	void
		OP_SyncCameras::pulsePressed(const char* name, void* reserved1)
	{
		if (strcmp(name, "Resync") == 0) {
			this->synchroniser.requestResync();
		}
	}

	//----------
	void
		OP_SyncCameras::getErrorString(OP_String* error, void* reserved1)
	{
		if (!this->errors.empty()) {
			std::string errorString;
			for (const auto& error : this->errors) {
				errorString += error.what() + "\n";
			}
			error->setString(errorString.c_str());
		}

		this->errors.clear();
	}
}
