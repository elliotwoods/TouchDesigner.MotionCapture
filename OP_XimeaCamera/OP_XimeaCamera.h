#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"
#include "GL_Extensions.h"

#include "CameraThread.h"
#include "MoCapLib.h"
#include "CameraParameters.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class OP_XimeaCamera : public DAT_CPlusPlusBase
	{
	public:
		OP_XimeaCamera(const OP_NodeInfo* info);
		virtual ~OP_XimeaCamera();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1) override;
	protected:
		void open(const OP_Inputs*);
		void close();

		std::shared_ptr<CameraThread> cameraThread;
		Links::Output output;

		Utils::ErrorBuffer errorBuffer;

		bool needsReopen = false;

		CameraParameters parameters;
	};
}
