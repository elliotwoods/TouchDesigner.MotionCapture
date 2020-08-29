#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"
#include "GL_Extensions.h"

#include "MoCapLib.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class DAT_Debug : public DAT_CPlusPlusBase
	{
	public:
		DAT_Debug(const OP_NodeInfo* info);
		virtual ~DAT_Debug();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1) override;
	protected:
		std::vector<Exception> errors;
	};
}
