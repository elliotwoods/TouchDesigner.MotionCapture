#include "pch_MoCapLib.h"
#include "Parameter.h"

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
		std::string
			AbstractParameter::getTDShortName() const
		{
			auto shortName = this->getName();
			if (shortName.empty()) {
				return shortName;
			}

			std::remove_if(shortName.begin(), shortName.end(), std::isspace);
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
		void
			ValueParameter<std::string>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = std::string(inputs->getParString(shortName.c_str()));
			auto valueHere = this->getValue();
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
			}
		}

		//----------
		template<>
		void
			ValueParameter<bool>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = (bool)inputs->getParInt(shortName.c_str(), 0);
			auto valueHere = this->getValue() ? 1 : 0;
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
			}
		}

		//----------
		template<>
		void
			ValueParameter<float>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = (float)inputs->getParDouble(shortName.c_str(), 0);
			auto valueHere = this->getValue();
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
			}
		}

		//----------
		template<>
		void
			ValueParameter<int>::updateFromInterface(const OP_Inputs* inputs)
		{
			auto shortName = this->getTDShortName();
			auto valueInTD = inputs->getParInt(shortName.c_str(), 0);
			auto valueHere = this->getValue();
			if (valueInTD != valueHere) {
				this->setValue(valueInTD);
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

			param.defaultValue = this->getDefaultValue().c_str();

			std::vector<const char*> optionsC;
			const auto& options = this->getOptions();
			for (const auto& option : options) {
				optionsC.push_back(option.c_str());
			}

			auto res = manager->appendMenu(param, options.size(), optionsC.data(), optionsC.data());
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
		void
			ParameterList::updateFromInterface(const OP_Inputs * inputs)
		{
			for (auto parameter : *this) {
				parameter->updateFromInterface(inputs);
			}
		}
	}
}
