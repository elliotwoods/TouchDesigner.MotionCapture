#pragma once

#include <string>

namespace TD_MoCap {
	namespace Links {

		class TDMOCAP_API BaseFrame {
		public:
			virtual std::string getTypeName() const = 0;
		};
	}
}