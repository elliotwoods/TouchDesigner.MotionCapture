#include "pch_MoCapLib.h"
#include "ChannelSet.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		void
			ChannelSet::setChannels(const std::initializer_list<std::string>& names)
		{
			this->resize(names.size());
			
			auto channelIt = this->begin();

			for (const auto & name : names)
			{
				channelIt->name = name;
				channelIt++;
			}
		}

		//----------
		void
			ChannelSet::setSampleCount(size_t size)
		{
			for (auto& channel : *this)
			{
				channel.samples.resize(size);
			}
		}

		//----------
		size_t
			ChannelSet::getSampleCount() const
		{
			if (this->empty()) {
				return 0;
			}
			else {
				return this->front().samples.size();
			}
		}
	}
}