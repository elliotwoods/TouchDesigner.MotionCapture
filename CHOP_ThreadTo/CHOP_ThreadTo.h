#pragma once
#include "CHOP_CPlusPlusBase.h"

namespace TD_MoCap {
	class CHOP_ThreadTo : public CHOP_CPlusPlusBase
	{
	public:
		CHOP_ThreadTo(const OP_NodeInfo* info);
		virtual ~CHOP_ThreadTo();

		void getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void* reserved1) override;
		bool getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void* reserved1) override;
		void getChannelName(int32_t index, OP_String*, const OP_Inputs*, void* reserverd) override;

		void execute(CHOP_Output*,
			const OP_Inputs*,
			void* reserved) override;

		void setupParameters(OP_ParameterManager* manager, void* reserved) override;
		void pulsePressed(const char* name, void* reserved1) override;
		void getErrorString(OP_String* error, void* reserved1) override;
	protected:
		Links::Input input;
		
		bool outputEmpty = true;
		bool previewDirty = true;

		Utils::ChannelSet channelSet;
		Utils::ErrorBuffer errorBuffer;
	};
}