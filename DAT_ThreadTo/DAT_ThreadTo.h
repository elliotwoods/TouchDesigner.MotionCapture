#pragma once
#include "DAT_CPlusPlusBase.h"

namespace TD_MoCap {
	class DAT_ThreadTo : public DAT_CPlusPlusBase
	{
	public:
		DAT_ThreadTo(const OP_NodeInfo* info);
		virtual ~DAT_ThreadTo();

		void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;
		void		execute(DAT_Output*,
			const OP_Inputs*,
			void* reserved) override;

		void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		void		pulsePressed(const char* name, void* reserved1) override;
		void		getErrorString(OP_String* error, void* reserved1) override;

	protected:
		Links::Input input;
		std::shared_ptr<Frames::BaseFrame> lastFrameReceived;
		Utils::ErrorBuffer errorBuffer;

		bool previewDirty = false;
	};
}