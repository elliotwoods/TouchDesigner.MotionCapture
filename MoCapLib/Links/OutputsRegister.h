#pragma once

#include "Output.h"

namespace TD_MoCap {
	namespace Links {
		class TDMOCAP_API OutputsRegister
		{
		public:
			static OutputsRegister& X();
			void add(Output*);
			void remove(Output*);
			Output* findByID(Output::ID);
		private:
			OutputsRegister() {}
			static std::unique_ptr<OutputsRegister> singleton;
			std::map<Output::ID, Output*> outputs;
		};
	}
}