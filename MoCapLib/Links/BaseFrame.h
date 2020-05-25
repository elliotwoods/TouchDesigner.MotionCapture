#pragma once

#include <string>
#include "Utils/Table.h"

namespace TD_MoCap {
	namespace Links {

		class TDMOCAP_API BaseFrame
		{
		public:
			virtual ~BaseFrame() {

			}

			virtual std::string getTypeName() const = 0;

			virtual bool getPreviewImage(cv::Mat&) const {
				return false;
			}

			virtual bool getPreviewDAT(Utils::Table&) const {
				return false;
			}
		};
	}
}