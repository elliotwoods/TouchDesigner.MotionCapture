#pragma once
#include "SOP_CPlusPlusBase.h"

namespace TD_MoCap {
	class SOP_ThreadTo : public SOP_CPlusPlusBase
	{
	public:
		SOP_ThreadTo(const OP_NodeInfo* info);
		virtual ~SOP_ThreadTo();

		void		getGeneralInfo(SOP_GeneralInfo*, const OP_Inputs*, void*) override;

		void		execute(SOP_Output*,
			const OP_Inputs*,
			void* reserved1) override;
		void		executeVBO(SOP_VBOOutput*
			, const OP_Inputs*
			, void* reserved1) override;

		void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		void		pulsePressed(const char* name, void* reserved1) override;

		void		getErrorString(OP_String* error, void* reserved1);
	protected:
		Links::Input input;
		
		std::vector<Exception> errors;
	};
}