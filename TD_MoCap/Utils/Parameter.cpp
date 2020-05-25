#include "pch_TD_MoCap.h"
#include "Parameter.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		const std::string& AbstractParameter::getName() const {
			return this->name;
		}

		//----------
		const std::string& AbstractParameter::getUnits() const {
			return this->units;
		}

		//----------
		ParameterList::ParameterList(std::initializer_list<AbstractParameter *> parameters) {
			for (auto& parameter : parameters) {
				this->push_back(parameter);
			}
		}
	}
}
