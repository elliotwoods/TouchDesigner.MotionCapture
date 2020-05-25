#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"

#include "MoCapLib.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class SyncCameras : public DAT_CPlusPlusBase
	{
	public:
		struct Camera {
			Links::Input input;
		};

		SyncCameras(const OP_NodeInfo* info);
		virtual ~SyncCameras();

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
		Links::Output output;
		std::map<Links::Output::ID, std::unique_ptr<Camera>> cameras;

		std::mutex errorsLock;
		std::vector<Exception> errors;
	};
}
