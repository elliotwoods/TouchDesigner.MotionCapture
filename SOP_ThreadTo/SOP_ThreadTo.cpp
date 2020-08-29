#include "pch_SOP_ThreadTo.h"
#include "SOP_ThreadTo.h"

namespace TD_MoCap {
	//----------
	SOP_ThreadTo::SOP_ThreadTo(const OP_NodeInfo* info)
	{
	}

	//----------
	SOP_ThreadTo::~SOP_ThreadTo()
	{
	}

	//----------
	void
		SOP_ThreadTo::getGeneralInfo(SOP_GeneralInfo* gInfo, const OP_Inputs* inputs, void*)
	{
		gInfo->cookEveryFrame = true;
		gInfo->directToGPU = false;
	}

	//----------
	void
		SOP_ThreadTo::execute(SOP_Output* output,
			const OP_Inputs* inputs,
			void* reserved1)
	{
		try {
			this->errorBuffer.updateFromInterface(inputs);

			// update input stats
			this->input.update(inputs->getParDAT("Source"));

			auto frame = this->input.receiveLatestFrame(true);
			if (frame) {
				frame->getPreviewSOP(output);
			}
		}
		catch (const Exception& e)
		{
			this->errorBuffer.push(e);
		}
	}

	//----------
	void
		SOP_ThreadTo::executeVBO(SOP_VBOOutput* output,
			const OP_Inputs* inputs,
			void* reserved1)
	{
		try {
			this->errorBuffer.updateFromInterface(inputs);

			// update input stats
			this->input.update(inputs->getParDAT("Source"));
			throw(Exception("VBO mode not supported"));
		}
		catch (const Exception& e)
		{
			this->errorBuffer.push(e);
		}
	}

	//----------
	void
		SOP_ThreadTo::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		{
			OP_StringParameter param;

			param.name = "Source";
			param.label = param.name;

			auto res = manager->appendDAT(param);
			assert(res == OP_ParAppendResult::Success);
		}

		this->errorBuffer.setupParameters(manager);
	}

	//----------
	void
		SOP_ThreadTo::pulsePressed(const char* name, void* reserved1)
	{
		this->errorBuffer.pulsePressed(name);
	}

	//---------
	void
		SOP_ThreadTo::getErrorString(OP_String* error, void*)
	{
		this->errorBuffer.getErrorString(error);
	}
}