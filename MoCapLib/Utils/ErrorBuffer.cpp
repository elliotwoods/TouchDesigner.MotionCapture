#include "pch_MoCapLib.h"
#include "ErrorBuffer.h"

#include <sstream>

namespace TD_MoCap {
	namespace Utils {
		//----------
		void
			ErrorBuffer::setupParameters(OP_ParameterManager* parameterManager)
		{
			this->maxErrors.populateInterface(parameterManager);
			this->printToConsole.populateInterface(parameterManager);

			{
				OP_NumericParameter param;
				param.label = "Clear errors";
				param.name = "Clearerror";
				parameterManager->appendPulse(param);
			}
		}

		//----------
		void
			ErrorBuffer::updateFromInterface(const OP_Inputs* inputs)
		{
			this->maxErrors.updateFromInterface(inputs);
			this->printToConsole.updateFromInterface(inputs);
		}

		//----------
		void
			ErrorBuffer::pulsePressed(const char* name)
		{
			if (strcmp(name, "Clearerror") == 0) {
				this->clear();
			}
		}

		//----------
		void
			ErrorBuffer::getErrorString(OP_String* errorString)
		{
			std::stringstream ss;
			
			for (const auto& error : this->errors)
			{
				ss << std::endl << error.what();
			}

			if (this->overflow) {
				ss << std::endl << "...";
			}

			errorString->setString(ss.str().c_str());
		}

		//----------
		void
			ErrorBuffer::push(const Exception& exception)
		{
			if (this->errors.size() < this->maxErrors.getValue())
			{
				this->errors.emplace_back(exception);
			}
			else {
				this->overflow = true;
			}

			if (this->printToConsole.getValue())
			{
				std::cerr << exception.what() << std::endl;
			}
		}

		//----------
		void
			ErrorBuffer::push(ThreadChannel<Exception>& channel)
		{
			Exception exception;
			while (channel.tryReceive(exception)) {
				this->push(exception);
			}
		}

		//----------
		void
			ErrorBuffer::clear()
		{
			this->errors.clear();
			this->overflow = false;
		}
	}
}
