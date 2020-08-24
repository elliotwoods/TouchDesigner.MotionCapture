#include "pch_MoCapLib.h"
#include "Parameter.h"

#include <cctype>

//issue with name clash
bool is_space(char ch) {
	return isspace(ch) ? true : false;
}

namespace TD_MoCap {
	namespace Utils {
#pragma mark AbstractParameter
		//----------
		const std::string&
			AbstractParameter::getName() const
		{
			return this->name;
		}

		//----------
		std::string makeShortName(const std::string& longName)
		{
			auto shortName = longName;
			if (shortName.empty()) {
				return shortName;
			}

			shortName.erase(std::remove_if(shortName.begin(), shortName.end(), is_space), shortName.end());
			std::transform(shortName.begin(), shortName.end(), shortName.begin(),
				[](char c) {
				return std::tolower(c);
			});

			shortName[0] = std::toupper(shortName[0]);
			if (shortName.size() > 10) {
				shortName.resize(10);
			}

			return shortName;
		}

		//----------
		std::string
			AbstractParameter::getTDShortName() const
		{
			return makeShortName(this->getName());
		}

#pragma mark ValueParameter
		//----------
		template<>
		void
			ValueParameter<bool>::populateInterface(OP_ParameterManager* manager) const
		{
			OP_NumericParameter param;

			auto shortName = this->getTDShortName();
			param.name = shortName.c_str();

			param.label = this->getName().c_str();

			param.defaultValues[0] = this->getDefaultValue() ? 1 : 0;

			auto res = manager->appendToggle(param);
			assert(res == OP_ParAppendResult::Success);
		}

		//----------
		template<>
		void
			ValueParameter<std::string>::populateInterface(OP_ParameterManager* manager) const
		{
			OP_StringParameter param;

			auto shortName = this->getTDShortName();
			param.name = shortName.c_str();

			param.label = this->getName().c_str();

			param.defaultValue = this->getDefaultValue().c_str();
			
			auto res = manager->appendString(param);
			assert(res == OP_ParAppendResult::Success);
		}

		//----------
		template<>
		bool
			ValueParameter<std::string>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = std::string(inputs->getParString(shortName.c_str()));
			auto valueHere = this->getValue();
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
				return true;
			}
			else {
				return false;
			}
		}

		//----------
		template<>
		bool
			ValueParameter<bool>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = (bool)inputs->getParInt(shortName.c_str(), 0);
			auto valueHere = this->getValue() ? 1 : 0;
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
				return true;
			}
			else {
				return false;
			}
		}

		//----------
		template<>
		bool
			ValueParameter<float>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = (float)inputs->getParDouble(shortName.c_str(), 0);
			auto valueHere = this->getValue();
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
				return true;
			}
			else {
				return false;
			}
		}

		//----------
		template<>
		bool
			ValueParameter<int>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = inputs->getParInt(shortName.c_str(), 0);
			auto valueHere = this->getValue();
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
				return true;
			}
			else {
				return false;
			}
		}

#pragma mark NumberParameter
		//----------
		template<>
			void
			NumberParameter<float>::populateInterface(OP_ParameterManager* manager) const
		{
			OP_NumericParameter param;

			auto shortName = this->getTDShortName();
			param.name = shortName.c_str();

			std::string label = this->getName();
			{
				auto units = this->getUnits();
				if (!units.empty()) {
					label += " [" + units + "]";
				}
			}
			param.label = label.c_str();

			param.defaultValues[0] = this->getDefaultValue();
			param.minSliders[0] = this->getSliderMin();
			param.maxSliders[0] = this->getSliderMax();
			param.minValues[0] = this->getMin();
			param.maxValues[0] = this->getMax();

			auto minMaxEnabled = this->getMax() > this->getMin();
			param.clampMins[0] = minMaxEnabled;
			param.clampMaxes[0] = minMaxEnabled;

			auto res = manager->appendFloat(param);
			assert(res == OP_ParAppendResult::Success);
		}

		//----------
		template<>
		void
			NumberParameter<int>::populateInterface(OP_ParameterManager* manager) const
		{
			OP_NumericParameter param;

			auto shortName = this->getTDShortName();
			param.name = shortName.c_str();

			std::string label = this->getName();
			{
				auto units = this->getUnits();
				if (!units.empty()) {
					label += " [" + units + "]";
				}
			}
			param.label = label.c_str();

			param.defaultValues[0] = this->getDefaultValue();
			param.minSliders[0] = this->getSliderMin();
			param.maxSliders[0] = this->getSliderMax();
			param.minValues[0] = this->getMin();
			param.maxValues[0] = this->getMax();

			auto minMaxEnabled = this->getMax() > this->getMin();
			param.clampMins[0] = minMaxEnabled;
			param.clampMaxes[0] = minMaxEnabled;

			auto res = manager->appendInt(param);
			assert(res == OP_ParAppendResult::Success);
		}

#pragma mark SelectorParameter
		//----------
		void
			SelectorParameter::populateInterface(OP_ParameterManager* manager) const
		{
			OP_StringParameter param;

			auto shortName = this->getTDShortName();
			param.name = shortName.c_str();

			param.label = this->getName().c_str();

			auto defaultValueName = makeShortName(this->getDefaultValue());
			param.defaultValue = defaultValueName.c_str();

			std::vector<const char*> optionNames;
			std::vector<const char*> optionLabels;
			const auto& options = this->getOptions();
			std::vector<std::string> shortOptionsCache;
			shortOptionsCache.reserve(options.size());
			for (const auto& option : options) {
				{
					auto shortName = makeShortName(option);
					shortOptionsCache.push_back(shortName);
				}

				optionNames.push_back(shortOptionsCache.back().c_str());
				optionLabels.push_back(option.c_str());
			}

			auto res = manager->appendMenu(param
				, options.size()
				, optionNames.data()
				, optionLabels.data());

			// note that all options must be valid TD names for now
			assert(res == OP_ParAppendResult::Success);
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
		const std::vector<std::string>&
			SelectorParameter::getOptions() const
		{
			return this->options;
		}

		//----------
		std::string
			SelectorParameter::getSelectedOption() const
		{
			for (const auto& option : this->options) {
				if (makeShortName(option) == this->getValue()) {
					return option;
				}
			}

			// consider throwing an exception here instead
			return "";
		}

#pragma mark PathParameter
		//----------
		PathParameter::PathParameter(const std::string& name, PathParameter::Type type)
			: ValueParameter<std::string>(name, "", "")
		{
			this->type = type;
		}

		//----------
		void
			PathParameter::populateInterface(OP_ParameterManager* manager) const
		{
			OP_StringParameter param;
			auto shortName = this->getTDShortName();
			param.name = shortName.c_str();
			param.label = this->getName().c_str();

			switch (this->type) {
			case Type::File:
				manager->appendFile(param);
				break;
			case Type::Folder:
				manager->appendFolder(param);
				break;
			}
		}

		//----------
		std::filesystem::path
			PathParameter::getPath() const
		{
			return std::filesystem::path(this->getValue());
		}


#pragma mark ParameterList
		//----------
		ParameterList::ParameterList(std::initializer_list<AbstractParameter*> parameters)
		{
			for (auto& parameter : parameters) {
				this->push_back(parameter);
			}
		}

		//----------
		void
			ParameterList::populateInterface(OP_ParameterManager* manager) const
		{
			for (auto parameter : *this) {
				parameter->populateInterface(manager);
			}
		}

		//----------
		bool
			ParameterList::updateFromInterface(const OP_Inputs * inputs)
		{
			auto anyChanged = false;
			for (auto parameter : *this) {
				anyChanged |= parameter->updateFromInterface(inputs);
			}
			return anyChanged;
		}
	}
}
