#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"

#include "MoCapLib.h"
#include "Synchroniser.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class OP_SyncCameras : public DAT_CPlusPlusBase
	{
	public:
		OP_SyncCameras(const OP_NodeInfo* info);
		virtual ~OP_SyncCameras();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1);
	protected:
		Synchroniser synchroniser;

		Utils::ErrorBuffer errorBuffer;
	};
}
