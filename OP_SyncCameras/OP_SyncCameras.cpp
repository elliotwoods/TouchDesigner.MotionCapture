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
		try {
			// update parameters
			this->errorBuffer.updateFromInterface(inputs);
			this->synchroniser.getParameters().updateFromInterface(inputs);

			// gather new IDs and check our list
			{
				// gather the list of ID's
				auto inputCount = inputs->getNumInputs();
				std::vector<Links::Output::ID> newOutputIDs;
				for (int i = 0; i < inputCount; i++) {
					Links::Output::ID id;
					if (Links::Output::getIDFromDAT(inputs->getInputDAT(i), id)) {
						newOutputIDs.push_back(id);
					}
				}

				// send the new ID list to the synchroniser
				this->synchroniser.checkConnections(newOutputIDs);
			}

			// Update output
			this->synchroniser.output.update();
			this->synchroniser.output.populateMainThreadOutput(output);

			// clear all our errors when we have a successful sync
			// This is because TD will bypass the OP if the error string is not empty
			{
				bool success;
				while (this->synchroniser.syncSuccess.tryReceive(success)) {
					this->errorBuffer.clear();

					//clear out exceptions from thread
					this->synchroniser.getThread().exceptionsInThread.clear();
				}
			}
		}
		catch (const Exception & e) {
			this->errorBuffer.push(e);
		}

		// gather any exceptions from the synchronsier thread
		this->errorBuffer.push(this->synchroniser.getThread().exceptionsInThread);
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

		this->errorBuffer.setupParameters(manager);

	}

	//----------
	void
		OP_SyncCameras::pulsePressed(const char* name, void* reserved1)
	{
		if (strcmp(name, "Resync") == 0) {
			this->synchroniser.requestResync();
		}
		this->errorBuffer.pulsePressed(name);
	}

	//----------
	void
		OP_SyncCameras::getErrorString(OP_String* error, void* reserved1)
	{
		this->errorBuffer.getErrorString(error);
	}
}
