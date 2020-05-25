#include "pch_TOP_ThreadTo.h"
#include "TOP_ThreadTo.h"

namespace TD_MoCap {
	//----------
	TOP_ThreadTo::TOP_ThreadTo(const OP_NodeInfo* info)
	{
	}

	//----------
	TOP_ThreadTo::~TOP_ThreadTo()
	{
	}

	//----------
	void
		TOP_ThreadTo::getGeneralInfo(TOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void*)
	{
		ginfo->cookEveryFrame = true;
		ginfo->memFirstPixel = TOP_FirstPixel::TopLeft;
		ginfo->memPixelType = OP_CPUMemPixelType::R8Fixed;
	}

	//----------
	bool
		TOP_ThreadTo::getOutputFormat(TOP_OutputFormat* format, const OP_Inputs* inputs, void* reserved1)
	{
		if (this->lastFrameReceived) {
			cv::Mat image;
			if (this->lastFrameReceived->getPreviewImage(image)) {
				setTOP_OutputFormat(format, image);

				// make a copy for comparison later
				this->allocatedOutputFormat = *format;
				return true;
			}
		}

		// in all other cases
		return false;
	}

	//----------
	void
		TOP_ThreadTo::execute(TOP_OutputFormatSpecs* output,
			const OP_Inputs* inputs,
			TOP_Context* context,
			void* reserved1)
	{
		this->input.update(inputs);

		if (!this->input.channel.empty()) {
			while (this->input.channel.tryReceive(this->lastFrameReceived)) {
				// flush frames until most recent
				this->previewDirty = true;
			}
		}

		if (this->previewDirty && this->lastFrameReceived) {
			cv::Mat image;
			if (!this->lastFrameReceived->getPreviewImage(image)) {
				// this frame doesn't support image output
				return;
			}

			if (image.empty()) {
				return;
			}

			TOP_OutputFormat requiredOutputFormat = this->allocatedOutputFormat; // initialise all elements the same that we don't care about
			setTOP_OutputFormat(&requiredOutputFormat, image);
			if (memcmp(&requiredOutputFormat, &this->allocatedOutputFormat, sizeof(requiredOutputFormat))) {
				// wait until next frame
				return;
			}

			// copy it into the gpu memory
			output->newCPUPixelDataLocation = 0;
			memcpy(output->cpuPixelData[0]
				, image.data
				, image.total() * image.elemSize());
		}
	}

	//----------
	void
		TOP_ThreadTo::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		this->input.setupParameters(manager);
	}

	//----------
	void
		TOP_ThreadTo::pulsePressed(const char* name, void* reserved1)
	{
	}
}