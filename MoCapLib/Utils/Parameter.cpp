#include "pch_MoCapLib.h"
#include "Parameter.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		const std::string&
			AbstractParameter::getName() const
		{
			return this->name;
		}

		//----------
		ParameterList::ParameterList(std::initializer_list<AbstractParameter *> parameters)
		{
			for (auto& parameter : parameters) {
				this->push_back(parameter);
			}
		}
	}
}
