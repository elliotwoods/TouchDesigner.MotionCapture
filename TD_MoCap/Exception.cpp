#include "pch_TD_MoCap.h"
#include "Exception.h"

namespace TD_MoCap {
	//----------
	Exception::Exception() {
	}

	//----------
	Exception::Exception(const char* message) {
		this->message = std::string(message);
	}

	//----------
	Exception::Exception(const std::string& message) {
		this->message = message;
	}
}