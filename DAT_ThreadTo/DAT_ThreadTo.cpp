#include "pch_DAT_ThreadTo.h"
#include "DAT_ThreadTo.h"

namespace TD_MoCap {
	//----------
	DAT_ThreadTo::DAT_ThreadTo(const OP_NodeInfo* info)
	{
	}

	//----------
	DAT_ThreadTo::~DAT_ThreadTo()
	{
	}

	//----------
	void
		DAT_ThreadTo::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void*)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		DAT_ThreadTo::execute(DAT_Output* output,
			const OP_Inputs* inputs,
			void* reserved)
	{
		try {
			this->errorBuffer.updateFromInterface(inputs);

			if (inputs->getNumInputs() < 1) {
				return;
			}

			this->input.update(inputs->getInputDAT(0));

			{
				auto newFrame = this->input.receiveLatestFrame(false);
				if (newFrame) {
					this->previewDirty = true;
					this->lastFrameReceived = newFrame;
				}
			}

			if (this->previewDirty && this->lastFrameReceived) {
				Utils::Table outputTable;
				this->lastFrameReceived->getPreviewDAT(outputTable);
				outputTable.populateOutput(output);
				this->previewDirty = false;
			}
		}
		catch (const Exception& e) {
			this->errorBuffer.push(e);
		}
	}

	//----------
	void
		DAT_ThreadTo::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		this->errorBuffer.setupParameters(manager);
	}

	//----------
	void
		DAT_ThreadTo::pulsePressed(const char* name, void* reserved1)
	{
		this->errorBuffer.pulsePressed(name);
	}

	//----------
	void
		DAT_ThreadTo::getErrorString(OP_String* error, void* reserved1)
	{
		this->errorBuffer.getErrorString(error);
	}
}