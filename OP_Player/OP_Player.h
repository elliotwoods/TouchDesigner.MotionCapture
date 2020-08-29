#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"

#include "MoCapLib.h"

#include "Player.h"

namespace TD_MoCap {
	class OP_Player : public DAT_CPlusPlusBase
	{
	public:
		OP_Player(const OP_NodeInfo* info);
		virtual ~OP_Player();

		virtual void		getGeneralInfo(DAT_GeneralInfo*, const OP_Inputs*, void* reserved1) override;

		virtual void		execute(DAT_Output*,
								const OP_Inputs*,
								void* reserved) override;

		bool		getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1) override;
		void		getInfoDATEntries(int32_t index,
			int32_t nEntries,
			OP_InfoDATEntries* entries,
			void* reserved1) override;

		virtual void		setupParameters(OP_ParameterManager* manager, void* reserved1) override;
		virtual void		pulsePressed(const char* name, void* reserved1) override;

		virtual void		getErrorString(OP_String* error, void* reserved1);
	protected:
		Utils::ErrorBuffer errorBuffer;

		struct {
			Utils::NumberParameter<float> framerate{
				"Framerate", "fps"
				, 50.0f, 50.0f
				, 0.1f, 1000.0f
			};

			Utils::NumberParameter<int> threads{
				"Threads", ""
				, 4, 4
				, 1, 64
			};

			Utils::NumberParameter<int> buffer{
				"Buffer", "frames"
				, 8, 8
				, 1, 100
			};

			Utils::ValueParameter<bool> play{
				"Play"
				, false, false
			};

			Utils::ValueParameter<bool> holdFrame{
				"Hold frame"
				, false, false
			};

			Utils::ParameterList list{
				&framerate
				, &threads
				, &buffer
				, &play
				, &holdFrame
			};

		} parameters;

		Utils::Table infoDAT;
		Links::Output output;
		std::unique_ptr<Player> player;
	};
}
