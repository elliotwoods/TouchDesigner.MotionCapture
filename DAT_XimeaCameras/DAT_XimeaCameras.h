#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"
#include "GL_Extensions.h"

#include "MoCapLib.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class DAT_XimeaCameras : public DAT_CPlusPlusBase
	{
	public:
		struct DeviceInfo {
			DeviceInfo();

			enum { size = 100};
			char serialNumber[size];
			char name[size];
			char instancePath[size];
			char locationPath[size];
			char type[size];
		};

		DAT_XimeaCameras(const OP_NodeInfo* info);
		virtual ~DAT_XimeaCameras();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1) override;
	protected:
		Utils::ErrorBuffer errorBuffer;
		bool needsRefresh = true;

		std::vector<DeviceInfo> deviceInfos;
	};
}
