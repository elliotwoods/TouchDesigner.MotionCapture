#pragma once
#include "DAT_CPlusPlusBase.h"

namespace TD_MoCap {
	class DAT_ThreadTo : public DAT_CPlusPlusBase
	{
	public:
		DAT_ThreadTo(const OP_NodeInfo* info);
		virtual ~DAT_ThreadTo();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;
		virtual void		execute(DAT_Output*,
			const OP_Inputs*,
			void* reserved) override;


	protected:
		Links::Input input;
		std::shared_ptr<Links::BaseFrame> lastFrameReceived;
		
		bool previewDirty = false;
	};
}