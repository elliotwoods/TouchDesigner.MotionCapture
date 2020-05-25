#include "pch_MoCapLib.h"
#include "OutputsRegister.h"

namespace TD_MoCap {
	namespace Links {
		//----------
		std::unique_ptr<OutputsRegister> OutputsRegister::singleton;

		//----------
		OutputsRegister &
			OutputsRegister::X()
		{
			if (!OutputsRegister::singleton) {
				OutputsRegister::singleton = std::unique_ptr<OutputsRegister>(new OutputsRegister());
			}
			return *OutputsRegister::singleton;
		}

		//----------
		void
			OutputsRegister::add(Output* output)
		{
			this->outputs.emplace(output->getID(), output);
		}

		//----------
		void
			OutputsRegister::remove(Output* output)
		{
			auto findOutput = this->outputs.find(output->getID());
			if (findOutput != this->outputs.end()) {
				this->outputs.erase(findOutput);
			}
		}

		//----------
		Output*
			OutputsRegister::findByID(Output::ID id)
		{
			auto findOutput = this->outputs.find(id);
			if (findOutput != this->outputs.end()) {
				return findOutput->second;
			}
			else {
				return nullptr;
			}
		}
	}
}