#include "pch_DAT_Debug.h"
#include "DAT_Debug.h"
#include <xiAPIplus/xiapiplus.h>

#include "Frames.h"

namespace TD_MoCap {
	//----------
	DAT_Debug::DAT_Debug(const OP_NodeInfo* info)
	{
	}

	//----------
	DAT_Debug::~DAT_Debug()
	{

	}

	//----------
	void
		DAT_Debug::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		DAT_Debug::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		try {
			this->errorBuffer.updateFromInterface(inputs);
			this->selectReport.updateFromInterface(inputs);

			if (this->selectReport.getValue() == "Frames")
			{
				debugFrames().populateOutput(output);
			}
			else {
				throw(Exception("Not supported"));
			}
		}
		catch (const Exception& e) {
			this->errorBuffer.push(e);
		}
	}

	//----------
	void
		DAT_Debug::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		this->selectReport.populateInterface(manager);
		this->errorBuffer.setupParameters(manager);
	}

	//----------
	void
		DAT_Debug::pulsePressed(const char* name, void* reserved1)
	{
		this->errorBuffer.pulsePressed(name);
	}

	//----------
	void
		DAT_Debug::getErrorString(OP_String* error, void* reserved1)
	{
		this->errorBuffer.getErrorString(error);
	}
}
