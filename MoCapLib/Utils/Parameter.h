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

			Event<void> onChange;

		protected:
			std::string name;

			// force dynamic casting to be enabled
			virtual void virtualFunction() { }
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

		protected:
			T value;
			T defaultValue;
		};

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

		protected:
			std::string units;
			T value;
			T defaultValue;
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

			const std::vector<std::string> & getOptions();
		protected:
			std::vector<std::string> options;
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
		};
	}
}