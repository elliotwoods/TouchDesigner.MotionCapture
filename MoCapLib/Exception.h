#pragma once

#include <string>

namespace TD_MoCap {
	class TDMOCAP_API Exception
	{
	public:
		Exception();
		Exception(const char*);
		Exception(const std::string&);
		virtual operator bool() const {
			return true;
		}

		const std::string& what() const;
	protected:
		std::string message;
	};

	class NoException : public Exception
	{
	public:
		operator bool() const override {
			return false;
		}
	};

	void TDMOCAP_API rethrowFormattedExceptions(const std::function<void()>&);
}
