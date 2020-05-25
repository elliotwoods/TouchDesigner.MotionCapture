#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"
#include "GL_Extensions.h"

#include "CameraThread.h"
#include "TD_MoCap.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class XimeaCamera : public DAT_CPlusPlusBase
	{
	public:
		XimeaCamera(const OP_NodeInfo* info);
		virtual ~XimeaCamera();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;

		virtual int32_t		getNumInfoCHOPChans(void* reserved1) override;
		virtual void		getInfoCHOPChan(int index,
			OP_InfoCHOPChan* chan,
			void* reserved1) override;

		virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1) override;
		virtual void		getInfoDATEntries(int32_t index,
			int32_t nEntries,
			OP_InfoDATEntries* entries,
			void* reserved1) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1);
	protected:
		void open(const OP_Inputs*);
		void close();

		std::shared_ptr<CameraThread> cameraThread;
		Links::Output output;

		std::mutex errorsLock;
		std::vector<Exception> errors;
		bool needsReopen = false;

		struct {
			Utils::ValueParameter <float> exposure{
				"Exposure", "ms"
				, 10, 10
				, 0, 10000
				, 0, 100
			};
			Utils::ParameterList list{ &exposure };
			std::set<Utils::AbstractParameter *> stale;
			std::mutex mutex;
		} parameters;
	};
}
