#include "pch_MoCapLib.h"
#include "TriangulateFrame.h"

namespace TD_MoCap {
	namespace Frames {
		//----------
		std::string
			TriangulateFrame::getTypeName() const
		{
			return "TriangulateFrame";
		}

		//----------
		uint64_t
			TriangulateFrame::getFrameIndex() const
		{
			return this->inputFrame->getFrameIndex();
		}
	}
}