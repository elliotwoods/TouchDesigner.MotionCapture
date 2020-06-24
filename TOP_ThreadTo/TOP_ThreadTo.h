#pragma once
#include "TOP_CPlusPlusBase.h"

namespace TD_MoCap {
	class TOP_ThreadTo : public TOP_CPlusPlusBase
	{
	public:
		TOP_ThreadTo(const OP_NodeInfo* info);
		virtual ~TOP_ThreadTo();

		virtual void		getGeneralInfo(TOP_GeneralInfo*, const OP_Inputs*, void*) override;
		virtual bool		getOutputFormat(TOP_OutputFormat*, const OP_Inputs*, void*) override;


		virtual void		execute(TOP_OutputFormatSpecs*,
			const OP_Inputs*,
			TOP_Context* context,
			void* reserved1) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1);
	protected:
		Links::Input input;
		
		TOP_OutputFormat allocatedOutputFormat;

		bool previewDirty = false;

		std::vector<Exception> errors;
	};
}