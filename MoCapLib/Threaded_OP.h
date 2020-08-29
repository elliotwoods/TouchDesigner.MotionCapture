#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"

#include "MoCapLib.h"

namespace TD_MoCap {
	template<typename InputFrameType, typename OutputFrameType, typename ParametersType>
	class IProcessor {
	public:
		using Parameters_t = ParametersType;
		using InputFrame_t = InputFrameType;
		using OutputFrame_t = OutputFrameType;
		virtual void process(std::shared_ptr<InputFrameType>, std::shared_ptr<OutputFrameType>, ParametersType&) = 0;
	};

	template<typename ProcessorType>
	class ThreadedOP : public DAT_CPlusPlusBase
	{
	public:
		ThreadedOP(const OP_NodeInfo* info);
		virtual ~ThreadedOP();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
			const OP_Inputs*,
			void* reserved) override;

		virtual int32_t		getNumInfoCHOPChans(void* reserved1) override;
		virtual void		getInfoCHOPChan(int index,
			OP_InfoCHOPChan* chan,
			void* reserved1) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1);
	protected:
		void process();
		Utils::WorkerThread thread;
		Utils::ErrorBuffer errorBuffer;
		std::string errorString;

		Links::Input input;
		Links::Output output;
		ProcessorType processor;

		std::mutex lockParameters;
		typename ProcessorType::Parameters_t parameters;
	};
}

#include "Threaded_OP.ipp"