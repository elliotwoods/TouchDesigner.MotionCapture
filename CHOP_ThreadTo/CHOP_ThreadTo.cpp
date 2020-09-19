#include "pch_CHOP_ThreadTo.h"
#include "CHOP_ThreadTo.h"

namespace TD_MoCap {
	//----------
	CHOP_ThreadTo::CHOP_ThreadTo(const OP_NodeInfo* info)
	{
	}

	//----------
	CHOP_ThreadTo::~CHOP_ThreadTo()
	{
	}

	//----------
	void
		CHOP_ThreadTo::getGeneralInfo(CHOP_GeneralInfo* info, const OP_Inputs*, void*)
	{
		info->cookEveryFrame = true;
		info->timeslice = false;
	}

	//----------
	bool
		CHOP_ThreadTo::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void*)
	{
		try {
			auto frameNow = this->input.receiveLatestFrame(false);
			if (frameNow) {
				this->previewDirty = true;

				// we have a new frame
				if (frameNow->getPreviewCHOP(this->channelSet)) {
					// it has channel data
					this->outputEmpty = this->channelSet.empty();
				}
				else {
					// it doesn't have channel data
					this->channelSet.clear();
					this->outputEmpty = true;
				}
			}

			info->numChannels = this->channelSet.size();
			info->numSamples = info->numChannels == 0 ? 0 : this->channelSet.front().samples.size();

			return true;
		}
		catch (Exception e) {
			this->errorBuffer.push(e);
			return false;
		}
	}

	//----------
	void
		CHOP_ThreadTo::getChannelName(int32_t index, OP_String* name, const OP_Inputs*, void* reserved)
	{
		if (index < this->channelSet.size()) {
			name->setString(this->channelSet[index].name.c_str());
		}
	}

	//----------
	void
		CHOP_ThreadTo::execute(CHOP_Output* output,
			const OP_Inputs* inputs,
			void* reserved)
	{
		try {
			this->errorBuffer.updateFromInterface(inputs);
			this->input.update(inputs->getParDAT("Source"));

			if (!this->outputEmpty && this->previewDirty) {
				auto sampleCount = MIN(this->channelSet.getSampleCount(), output->numSamples);

				for (size_t i = 0; i < this->channelSet.size(); i++) {
					output->names[i] = this->channelSet[i].name.c_str();
					memcpy(output->channels[i], this->channelSet[i].samples.data(), sizeof(float) * sampleCount);
				}
				this->previewDirty = false;
			}
		}
		catch (Exception e) {
			this->errorBuffer.push(e);
		}
		
	}

	//----------
	void
		CHOP_ThreadTo::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		{
			OP_StringParameter param;

			param.name = "Source";
			param.label = param.name;

			auto res = manager->appendDAT(param);
			assert(res == OP_ParAppendResult::Success);
		}
		this->errorBuffer.setupParameters(manager);
	}

	//----------
	void
		CHOP_ThreadTo::pulsePressed(const char* name, void* reserved1)
	{
		this->errorBuffer.pulsePressed(name);
	}

	//----------
	void
		CHOP_ThreadTo::getErrorString(OP_String* error, void*)
	{
		this->errorBuffer.getErrorString(error);
	}
}