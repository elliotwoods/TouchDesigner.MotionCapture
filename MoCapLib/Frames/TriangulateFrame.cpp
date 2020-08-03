#include "pch_MoCapLib.h"
#include "TriangulateFrame.h"

namespace TD_MoCap {
	namespace Frames {
		//----------
		TriangulateFrame::TriangulateFrame()
		{

		}

		//----------
		std::shared_ptr<TriangulateFrame>
			TriangulateFrame::make()
		{
			return std::shared_ptr<TriangulateFrame>(new TriangulateFrame);
		}

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

		//----------
		bool
			TriangulateFrame::getPreviewCHOP(Utils::ChannelSet& channelSet) const
		{
			channelSet.setChannels({ "x", "y", "z" });
			channelSet.setSampleCount(this->worldPoints.size());
			for (int c = 0; c < 3; c++) {
				for (int i = 0; i < this->worldPoints.size(); i++) {
					channelSet[c].samples[i] = this->worldPoints[i][c];
				}
			}
			return true;
		}
	}
}