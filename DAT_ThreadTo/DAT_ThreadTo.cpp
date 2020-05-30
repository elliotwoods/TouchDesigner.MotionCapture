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
}