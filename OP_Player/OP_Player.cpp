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
		auto folder = std::string(inputs->getParFilePath("Folder"));
		this->parameters.list.updateFromInterface(inputs);
		
		if (this->player) {
			if (!this->parameters.play.getValue()) {
				this->player.reset();
			}
			else {
				this->player->setFramerate(this->parameters.framerate.getValue());
				this->player->update();
			}
		}
		else {
			if (this->parameters.play.getValue()) {
				try {
					rethrowFormattedExceptions([&] {
						this->player = std::make_unique<Player>(folder
							, this->output
							, this->parameters.threads.getValue()
							, this->parameters.buffer.getValue());
					});
				}
				catch (const Exception& e) {
					this->player.reset();
					this->errors.push_back(e);
				}
			}
		}

		if (this->player) {
			this->infoDAT = this->player->getRecordingInfo();
		}
		else {
			this->infoDAT.clear();
		}

		// update enabled parameters
		{
			inputs->enablePar("Folder", !this->player);
			inputs->enablePar(this->parameters.play.getTDShortName().c_str(), !folder.empty() || this->player);
			inputs->enablePar(this->parameters.threads.getTDShortName().c_str(), !this->player);
			inputs->enablePar(this->parameters.buffer.getTDShortName().c_str(), !this->player);
		}

		this->output.update();
		this->output.populateMainThreadOutput(output);
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
	}

	//----------
	void
		OP_Player::pulsePressed(const char* name, void* reserved1)
	{

	}

	//----------
	void
		OP_Player::getErrorString(OP_String* error, void* reserved1)
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
