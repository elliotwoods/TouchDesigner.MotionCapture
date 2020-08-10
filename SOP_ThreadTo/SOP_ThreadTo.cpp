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
		// update input stats
		this->input.update(inputs->getParDAT("Source"));

		auto frame = this->input.receiveLatestFrame(true);
		if (frame) {
			frame->getPreviewSOP(output);
		}
	}

	//----------
	void
		SOP_ThreadTo::executeVBO(SOP_VBOOutput* output,
			const OP_Inputs* inputs,
			void* reserved1)
	{
		// update input stats
		this->input.update(inputs->getParDAT("Source"));
		this->errors.push_back(Exception("VBO mode not supported"));
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
	}

	//----------
	void
		SOP_ThreadTo::pulsePressed(const char* name, void* reserved1)
	{
	}

	//---------
	void
		SOP_ThreadTo::getErrorString(OP_String* error, void*)
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