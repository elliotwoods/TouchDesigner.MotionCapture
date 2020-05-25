#include "pch_MoCapLib.h"
#include "Input.h"
#include "OutputsRegister.h"

bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


namespace TD_MoCap {
	namespace Links {
		//----------
		Input::~Input()
		{
			this->unsubscribe();
		}

		//----------
		void
			Input::setupParameters(OP_ParameterManager* manager)
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
			Input::update(const OP_Inputs* inputs)
		{
			auto dat = inputs->getParDAT("Source");
			if(dat->numCols < 2 || dat->numRows < 1) {
				this->unsubscribe();
				return;
			}
			if (strcmp(dat->getCell(0, 0), "ID") != 0) {
				this->unsubscribe();
				return;
			}

			auto cellString = dat->getCell(0, 1);
			if (!is_number(cellString)) {
				this->unsubscribe();
				return;
			}
			auto newID = strtol(cellString, nullptr, 10);

			// if we stayed the same, ignore it
			if (this->connectedOutput) {
				if (connectedOutput->getID() == newID) {
					return;
				}
			}

			this->subscribe(newID);
		}

		//----------
		void
			Input::subscribe(Output::ID id)
		{
			this->unsubscribe();

			auto output = OutputsRegister::X().findByID(id);
			if (output) {
				this->connectedOutput = output;
				output->addSubscriber(this);
			}
		}

		//----------
		void
			Input::unsubscribe()
		{
			if (!this->connectedOutput) {
				return;
			}
			else {
				this->connectedOutput->removeSubscriber(this);
				this->connectedOutput = nullptr;
			}
		}
	}
}