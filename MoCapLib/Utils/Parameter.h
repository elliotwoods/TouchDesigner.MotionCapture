#pragma once

#include <string>

#include "Event.h"

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API AbstractParameter
		{
		public:
			AbstractParameter(const std::string& name)
				: name(name) {
			}
			const std::string& getName() const;
			std::string getTDShortName() const;

			virtual void populateInterface(OP_ParameterManager*) const = 0;
			virtual bool updateFromInterface(const OP_Inputs* inputs) = 0;

			Event<void> onChange;

		protected:
			std::string name;
		};

		template<typename T>
		class TDMOCAP_API ValueParameter : public AbstractParameter
		{
		public:
			ValueParameter(const std::string &name, const T& value, const T& defaultValue)
				: AbstractParameter(name)
				, value(value)
				, defaultValue(defaultValue)
			{ }

			const T& getValue() const
			{
				return this->value;
			}

			void setValue(const T& value)
			{
				this->value = value;
				this->onChange.notifyListeners();
			}

			const T& getDefaultValue() const
			{
				return this->defaultValue;
			}

			virtual void populateInterface(OP_ParameterManager*) const override {

			}

			virtual bool updateFromInterface(const OP_Inputs* inputs) override;

		protected:
			T value;
			T defaultValue;
		};

		// template overrides for specific populateInterface functions
		template<> void TDMOCAP_API ValueParameter<bool>::populateInterface(OP_ParameterManager*) const;
		template<> void TDMOCAP_API ValueParameter<std::string>::populateInterface(OP_ParameterManager*) const;
		template<> bool TDMOCAP_API ValueParameter<bool>::updateFromInterface(const OP_Inputs*);
		template<> bool TDMOCAP_API ValueParameter<std::string>::updateFromInterface(const OP_Inputs*);

		template<typename T>
		class TDMOCAP_API NumberParameter : public ValueParameter<T>
		{
		public:
			NumberParameter(const std::string & name, const std::string& units, T value = 0, T defaultValue = 0, T min = 0, T max = 0)
				: ValueParameter<T>(name, value, defaultValue)
				, units(units)
				, min(min)
				, max(max)
				, sliderMin(min)
				, sliderMax(max)
			{ }

			NumberParameter(const std::string& name, const std::string& units, T value, T defaultValue, T min, T max, T sliderMin, T sliderMax)
				: ValueParameter<T>(name, value, defaultValue)
				, units(units)
				, min(min)
				, max(max)
				, sliderMin(sliderMin)
				, sliderMax(sliderMax)
			{ }

			const std::string& getUnits() const
			{
				return this->units;
			}

			const T& getMin() const
			{
				return this->min;
			}

			const T& getMax() const
			{
				return this->max;
			}

			const T& getSliderMin() const
			{
				return this->sliderMin;
			}

			const T& getSliderMax() const
			{
				return this->sliderMax;
			}

			virtual void populateInterface(OP_ParameterManager*) const override;

		protected:
			std::string units;
			T min;
			T max;
			T sliderMin;
			T sliderMax;
		};

		class TDMOCAP_API SelectorParameter : public ValueParameter<std::string>
		{
		public:
			SelectorParameter(const std::string& name
				, const std::initializer_list<std::string>& options
				, const std::string& defaultValue);

			const std::vector<std::string> & getOptions() const;
			std::string getSelectedOption() const;

			virtual void populateInterface(OP_ParameterManager*) const override;

			//virtual void updateFromInterface(OP_Inputs* inputs) override;
			// we use the default ValueParameter<std::string>::updateFromInterface

		protected:
			std::vector<std::string> options;
		};

		class TDMOCAP_API PathParameter : public ValueParameter<std::string>
		{
		public:
			enum Type {
				Folder
				, File
			};

			PathParameter(const std::string& name, Type);

			void populateInterface(OP_ParameterManager*) const override;

			std::filesystem::path getPath() const;
		protected:
			Type type;
		};

		class TDMOCAP_API ParameterList : public std::vector<AbstractParameter*>
		{
		public:
			ParameterList(std::initializer_list<AbstractParameter *> parameters);
			
			template<typename T>
			T* operator[](const std::string& name) {
				for (auto parameter : *this) {
					if (parameter->getName() == name) {
						auto typedParameter = dynamic_cast<T*>(parameter);
						return typedParameter;
					}
				}
				return nullptr;
			}

			virtual void populateInterface(OP_ParameterManager*) const;
			virtual bool updateFromInterface(const OP_Inputs*);
		};
	}
}