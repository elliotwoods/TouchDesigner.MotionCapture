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
			this->errors.push_back(e);
		}
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
		{
			OP_NumericParameter pulse;
			pulse.name = "Clearexcept";
			pulse.label = "Clear exceptions";
			manager->appendPulse(pulse);
		}

		std::unique_lock<std::mutex> lockParameters(this->lockParameters);
		this->parameters.list.populateInterface(manager);
		
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::pulsePressed(const char* name, void* reserved1)
	{
		if (strcmp(name, "Clearexcept")) {
			this->errorString.clear();
		}
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::getErrorString(OP_String* error, void* reserved1)
	{
		Exception exception;
		while (this->thread.exceptionsInThread.tryReceive(exception)) {
			this->errorString += exception.what() + "\n";
		}
		for (const auto& error : this->errors) {
			this->errorString += error.what() + "\n";
		}
		this->errors.clear();

		error->setString(errorString.c_str());
	}

	//----------
	template<class ProcessorType>
	void
		ThreadedOP<ProcessorType>::process()
	{
		auto timeout = std::chrono::milliseconds(100);
		auto frame = this->input.receiveNextFrameWait(timeout);

		this->lockParameters.lock();
		auto parametersCopy = this->parameters;
		this->lockParameters.unlock();

		while (frame) {
			auto typedFrame = std::dynamic_pointer_cast<ProcessorType::InputFrame_t>(frame);
			if (typedFrame) {
				{
					std::unique_lock<std::mutex> lockParameters(this->lockParameters);
					if (this->parameters.needsUpdate) {
						this->parameters.update();
						parametersCopy = this->parameters;
						this->parameters.needsUpdate = false;
					}
				}

				auto outputFrame = ProcessorType::OutputFrame_t::make();
				outputFrame->inputFrame = typedFrame;
				outputFrame->startComputeTimer();
				this->processor.process(typedFrame, outputFrame, parametersCopy);
				outputFrame->endComputeTimer();

				this->output.send(outputFrame);
			}

			frame = this->input.receiveNextFrameWait(timeout);
		}
	}
}