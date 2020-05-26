#include "pch_MoCapLib.h"
#include "Exception.h"

namespace TD_MoCap {
	//----------
	Exception::Exception()
	{
	}

	//----------
	Exception::Exception(const char* message)
	{
		this->message = std::string(message);
	}

	//----------
	Exception::Exception(const std::string& message)
	{
		this->message = message;
	}

	//----------
	const std::string & 
		Exception::what() const {
		return this->message;
	}

	//----------
	void
		rethrowFormattedExceptions(const std::function<void()>& action)
	{
		try {
			action();
		}
		catch (const Exception&) {
			throw; // rethrow
		}
		catch (const std::exception& e) {
			throw(Exception(e.what()));
		}
		catch (...) {
			throw(Exception("Unknown exception"));
		}
	}
}