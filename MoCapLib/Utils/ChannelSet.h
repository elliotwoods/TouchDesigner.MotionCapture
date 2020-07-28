#pragma once

#include <string>
#include <map>
#include <vector>

namespace TD_MoCap {
	namespace Utils {
		struct Channel {
			std::string name;
			std::vector<float> samples;
		};

		class TDMOCAP_API ChannelSet : public std::vector<Channel> {
		public:
			void setChannels(const std::initializer_list<std::string>& names);
			void setSampleCount(size_t);
			size_t getSampleCount() const;
		};
	}
}