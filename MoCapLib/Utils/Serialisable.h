#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API Serialisable {
		public:
			struct Args {
				std::filesystem::path folderOut;
				uint64_t frameIndex = 0;
				std::string imageFormat;
				std::function<void()> onComplete;
			};

			virtual void serialise(nlohmann::json& json, const Args&) const {};
			virtual void deserialise(const nlohmann::json& json, const std::filesystem::path& workingFolder) {};
		};
	}
}