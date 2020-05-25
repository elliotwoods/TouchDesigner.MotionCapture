#pragma once

#include "Utils/Channel.h"
#include "Links/Output.h"

namespace TD_MoCap {
	namespace Links {
		class TDMOCAP_API Input
		{
		public:
			virtual ~Input();
			void update(const OP_DATInput*);

			void unsubscribe();
			void subscribe(Output::ID);
			
			Utils::Channel<std::shared_ptr<BaseFrame>> channel;
		protected:
			Output* connectedOutput = nullptr;
		};
	}
}
