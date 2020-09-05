#include "pch_OP_Player.h"
#include "OP_Player.h"

#include <time.h>
#include <codecvt>

namespace TD_MoCap {
	//----------
	OP_Player::OP_Player(const OP_NodeInfo* info)
	{

	}

	//----------
	OP_Player::~OP_Player()
	{
	}

	//----------
	void
		OP_Player::getGeneralInfo(DAT_GeneralInfo* info, const OP_Inputs*, void* reserved1)
	{
		info->cookEveryFrame = true;
	}

	//----------
	void
		OP_Player::execute(DAT_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		this->parameters.list.updateFromInterface(inputs);

		try {
			this->errorBuffer.updateFromInterface(inputs);

			if (this->player) {
				if (!this->parameters.play.getValue()) {
					this->player.reset();
				}
				else {
					this->player->setFramerate(this->parameters.framerate.getValue());
					this->player->setHoldFrame(this->parameters.holdFrame.getValue());
					this->player->update();
				}
			}
			else {
				if (this->parameters.play.getValue()) {
					try {
						auto folder = std::string(inputs->getParFilePath("Folder"));

						rethrowFormattedExceptions([&] {
							this->player = std::make_unique<Player>(folder
								, this->output);
						});
					}
					catch (const Exception& e) {
						this->player.reset();
						this->errorBuffer.push(e);
					}
				}
			}

			if (this->player) {
				this->infoDAT = this->player->getRecordingInfo();
			}
			else {
				this->infoDAT.clear();
			}

			// update enabled parameters (disable some during playing)
			{
				inputs->enablePar("Folder", !this->player);
			}

			this->output.update();
			this->output.populateMainThreadOutput(output);

			// observe any errors
			if (this->player) {
				this->errorBuffer.push(this->player->thread.exceptionsInThread);
			}
		}
		catch (const Exception& e)
		{
			this->errorBuffer.push(e);
		}
	}

	//----------
	bool
		OP_Player::getInfoDATSize(OP_InfoDATSize* infoSize, void*)
	{
		this->infoDAT.getInfoSize(infoSize);
		return true;
	}

	//----------
	void
		OP_Player::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void*)
	{
		this->infoDAT.populateInfoEntries(entries, index);
	}

	//----------
	void
		OP_Player::setupParameters(OP_ParameterManager* manager, void* reserved1)
	{
		// folder
		{
			OP_StringParameter param;

			param.name = "Folder";
			param.label = "Recording folder";
			
			auto res = manager->appendFolder(param);
			assert(res == OP_ParAppendResult::Success);
		}

		this->parameters.list.populateInterface(manager);

		this->errorBuffer.setupParameters(manager);
	}

	//----------
	void
		OP_Player::pulsePressed(const char* name, void* reserved1)
	{
		this->errorBuffer.pulsePressed(name);
	}

	//----------
	void
		OP_Player::getErrorString(OP_String* error, void* reserved1)
	{
		this->errorBuffer.getErrorString(error);
	}
}
