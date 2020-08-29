#pragma once

#include "Threaded_OP.h"

namespace TD_MoCap {
	//----------
	template<class ProcessorType>
	ThreadedOP<ProcessorType>::ThreadedOP(const OP_NodeInfo* info)
	{
		this->thread.setIdleFunction([this]() {
			this->process();
		});
	}

	//----------
	template<class ProcessorType>
	ThreadedOP<ProcessorType>::~ThreadedOP()
	{
		this->input.close();
		this->thread.join();
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		try {

			this->input.update(inputs->getInputDAT(0));

			{
				std::unique_lock<std::mutex> lockParameters(this->lockParameters);
				this->parameters.needsUpdate |= this->parameters.list.updateFromInterface(inputs);
			}

			this->output.update();
			this->output.populateMainThreadOutput(output);
		}
		catch (Exception e)
		{
			this->errorBuffer.push(e);
		}

		this->errorBuffer.push(this->thread.exceptionsInThread);
	}

	//----------
	template<class ProcessorType>
	int32_t
		ThreadedOP<ProcessorType>::getNumInfoCHOPChans(void* reserved1)
	{
		return 0;
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved1)
	{

	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		std::unique_lock<std::mutex> lockParameters(this->lockParameters);
		this->parameters.list.populateInterface(manager);
		
		this->errorBuffer.setupParameters(manager);
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::pulsePressed(const char* name, void* reserved1)
	{
		this->errorBuffer.pulsePressed(name);
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::getErrorString(OP_String* error, void* reserved1)
	{
		this->errorBuffer.getErrorString(error);
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::process()
	{
		auto timeout = std::chrono::milliseconds(100);
		auto frame = this->input.receiveNextFrameWait(timeout);

		if (!frame) {
			return;
		}

		auto typedFrame = std::dynamic_pointer_cast<ProcessorType::InputFrame_t>(frame);
		if (!typedFrame) {
			return;
		}

		this->lockParameters.lock();
		if (this->parameters.needsUpdate) {
			this->parameters.update();
			this->parameters.needsUpdate = false;
		}
		auto parametersCopy = this->parameters;
		this->lockParameters.unlock();

		auto outputFrame = ProcessorType::OutputFrame_t::make();
		outputFrame->inputFrame = typedFrame;
		outputFrame->startComputeTimer();
		{
			this->processor.process(typedFrame, outputFrame, parametersCopy);
		}
		outputFrame->endComputeTimer();

		this->output.send(outputFrame);
	}
}