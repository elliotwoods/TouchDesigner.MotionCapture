#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"

#include "MoCapLib.h"

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

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1);
	protected:
		void startRecording();
		void stopRecording();
		void updateRecording();

		Links::Input input;
		std::vector<Exception> errors;

		struct {
			Utils::NumberParameter<int> maxQueueLength{
				"Max queue length", "frames"
				, 1000, 1000
				, 1, 10000
				, 1, 10000
			};

			Utils::SelectorParameter format{
				"Image format"
				, {"Bmp", "Png", "Tif", "Jpeg", "Pbm"}
				, "Bmp"
			};

			Utils::ValueParameter<bool> timestampFolder{
				"Timestamp folder"
				, true, true
			};

			Utils::ValueParameter<bool> record{
				"Record"
				, false, false
			};

			Utils::ParameterList list{
				&maxQueueLength
				, &format
				, &timestampFolder
				, &record };

		} parameters;

		Utils::FrameRateCounter recordFrameRateCounter;
		bool isRecording = false;
		nlohmann::json recordingJson;
		std::filesystem::path outputPath;
		Utils::Serialisable::Args saveArgs;
	};
}
