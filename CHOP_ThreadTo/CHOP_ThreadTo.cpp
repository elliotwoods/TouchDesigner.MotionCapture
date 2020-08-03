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
		CHOP_ThreadTo::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs*, void*)
	{
		try {
			auto frameIsNew = (bool) this->input.receiveNextFrameDontWait();
			auto frame = this->input.getLastFrame();
			if (frame) {
				if (!frame->getPreviewCHOP(this->channelSet)) {
					this->channelSet.clear();
				}

				outputEmpty = this->channelSet.empty();
				info->numChannels = this->channelSet.size();
				info->numSamples = info->numChannels == 0 ? 0 : this->channelSet.front().samples.size();
				
				this->previewDirty = frameIsNew;
				return true;
			}
			else {
				return false;
			}

		}
		catch (Exception e) {
			this->errors.push_back(e);
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
			this->errors.push_back(e);
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
	}

	//----------
	void
		CHOP_ThreadTo::getErrorString(OP_String* error, void*)
	{
		if (!this->errors.empty()) {
			std::string errorString;
			for (const auto& error : this->errors) {
				errorString += error.what() + "\n";
			}
			error->setString(errorString.c_str());
		}
		this->errors.clear();
	}
}