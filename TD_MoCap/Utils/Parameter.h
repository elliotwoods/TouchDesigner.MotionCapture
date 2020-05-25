#pragma once

#include <string>

#include "Event.h"

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API AbstractParameter {
		public:
			AbstractParameter(const std::string& name, const std::string& units)
				: name(name)
				, units(units) {

			}

			const std::string& getName() const;
			const std::string& getUnits() const;

			Event<void> onChange;

		protected:
			std::string name;
			std::string units;

			// force dynamic casting to be enabled
			virtual void virtualFunction() { }
		};

		template<typename T>
		class TDMOCAP_API ValueParameter : public AbstractParameter {
		public:
			ValueParameter(const std::string & name, const std::string& units, T value = 0, T defaultValue = 0, T min = 0, T max = 0)
				: AbstractParameter(name, units)
				, value(value)
				, defaultValue(defaultValue)
				, min(min)
				, max(max)
				, sliderMin(min)
				, sliderMax(max) {
			}
			ValueParameter(const std::string& name, const std::string& units, T value, T defaultValue, T min, T max, T sliderMin, T sliderMax)
				: AbstractParameter(name, units)
				, value(value)
				, defaultValue(defaultValue)
				, min(min)
				, max(max)
				, sliderMin(sliderMin)
				, sliderMax(sliderMax) {
			}

			const T& getValue() const {
				return this->value;
			}

			void setValue(const T& value) {
				this->value = value;
				this->onChange.notifyListeners();
			}

			const T& getDefaultValue() const {
				return this->defaultValue;
			}

			const T& getMin() const {
				return this->min;
			}

			const T& getMax() const {
				return this->max;
			}

			const T& getSliderMin() const {
				return this->sliderMin;
			}

			const T& getSliderMax() const {
				return this->sliderMax;
			}

		protected:
			T value;
			T defaultValue;
			T min;
			T max;
			T sliderMin;
			T sliderMax;
		};

		template<>
		class TDMOCAP_API ValueParameter<std::string> : public AbstractParameter {
		public:
			std::string value;
			std::string defaultValue;
		};

		class TDMOCAP_API ParameterList : public std::vector<AbstractParameter*> {
		public:
			ParameterList(std::initializer_list<AbstractParameter *> parameters);
		};
	}
}