#pragma once

#include "BaseFrame.h"
#include "CentroidsFrame.h"

namespace TD_MoCap {
	namespace Frames {
		class TDMOCAP_API TriangulateFrame : public BaseFrame
		{
		public:
			std::string getTypeName() const;
			uint64_t getFrameIndex() const;
		
			std::shared_ptr<CentroidsFrame> inputFrame;
		};
	}
}
