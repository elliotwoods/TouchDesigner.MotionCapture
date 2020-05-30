#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"

#include "MoCapLib.h"

#include <memory>
#include <vector>
#include <set>

namespace TD_MoCap {
	class OP_Recorder : public DAT_CPlusPlusBase
	{
	public:
		OP_Recorder(const OP_NodeInfo* info);
		virtual ~OP_Recorder();

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
		Links::Input input;
		std::vector<Exception> errors;

		struct {
			Utils::NumberParameter<int> maxQueueLength{
				"Max queue length", "frames"
				, 100, 100
				, 1, 10000
				, 1, 1000
			};

			Utils::ValueParameter<bool> record{
				"Record"
				, false, false
			};

			Utils::ValueParameter<bool> play{
				"Play"
				, false, false
			};

			Utils::ParameterList list{ &maxQueueLength, &record, &play };
		} parameters;

		Utils::WorkerGroup workGroup;
	};
}
