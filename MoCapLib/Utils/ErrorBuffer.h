#pragma once


#include "../Exception.h"
#include "Parameter.h"
#include "Parameters.h"
#include "ThreadChannel.h"

#include "CPlusPlus_Common.h"

#include <queue>

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API ErrorBuffer {
		public:
			void setupParameters(OP_ParameterManager*);
			void updateFromInterface(const OP_Inputs*);
			void pulsePressed(const char* name);
			void getErrorString(OP_String* errorString);

			void push(const Exception&);
			void push(ThreadChannel<Exception>&);
			void clear();
		protected:
			std::deque<Exception> errors;
			bool overflow = false;

			NumberParameter<int> maxErrors
			{
				"Max errors", "errors"
				, 50, 50
				, 1, 10000
				, 1, 100
			};

			ValueParameter<bool> printToConsole
			{
				"Print to console"
				, false, false
			};
		};
	}
}