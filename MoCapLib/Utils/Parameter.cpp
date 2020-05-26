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

		//----------
		SelectorParameter::SelectorParameter(const std::string& name
			, const std::initializer_list<std::string>& options
			, const std::string& defaultValue)
		: ValueParameter<std::string>(name, defaultValue, defaultValue)
		, options(options)
		{

		}

		//----------
		const std::vector<std::string> &
			SelectorParameter::getOptions()
		{
			return this->options;
		}
	}
}
