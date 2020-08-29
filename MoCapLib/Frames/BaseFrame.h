#pragma once

#include <string>
#include "Utils/Table.h"
#include "Utils/Serialisable.h"
#include "Utils/ChannelSet.h"
#include "SOP_CPlusPlusBase.h"

namespace TD_MoCap {
	namespace Frames {

		class TDMOCAP_API BaseFrame : public Utils::Serialisable
		{
		public:
			BaseFrame();
			virtual ~BaseFrame();

			virtual std::string getTypeName() const;

			virtual bool getPreviewImage(cv::Mat&) const {
				return false;
			}

			virtual bool getPreviewDAT(Utils::Table&) const {
				return false;
			}

			virtual bool getPreviewCHOP(Utils::ChannelSet&) const {
				return false;
			}

			virtual bool getPreviewSOP(SOP_Output*) const {
				return false;
			}

			void startComputeTimer();
			void endComputeTimer();
			std::chrono::high_resolution_clock::duration getComputeTime() const;
			void setComputeTime(const std::chrono::high_resolution_clock::duration&);

			virtual uint64_t getFrameIndex() const = 0;

			static std::map<std::string, size_t> getCountPerType();
		protected:
			std::chrono::high_resolution_clock::time_point startCompute;
			std::chrono::high_resolution_clock::duration computeTime;
		private:
			static std::mutex lockAllFrames;
			static std::set<BaseFrame*> allFrames;
		};
	}
}