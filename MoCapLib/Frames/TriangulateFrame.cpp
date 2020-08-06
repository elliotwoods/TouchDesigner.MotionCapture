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
			channelSet.setChannels({
				"x", "y", "z"
				, "intersect_length"
				, "camera_1_x",  "camera_1_y"
				, "camera_2_x",  "camera_2_y"
				, "epipolar_distance", "mass_ratio", "angle_distance"
				});
			channelSet.setSampleCount(this->worldPoints.size());

			auto leaderID = this->inputFrame->inputFrame->leaderID;
			auto followerID = this->inputFrame->inputFrame->secondaryID;

			for (int i = 0; i < this->worldPoints.size(); i++) {
				int c = 0;
				channelSet[c++].samples[i] = this->worldPoints[i].x;
				channelSet[c++].samples[i] = this->worldPoints[i].y;
				channelSet[c++].samples[i] = this->worldPoints[i].z;
				channelSet[c++].samples[i] = this->intersections[i].getLength();

				channelSet[c++].samples[i] = this->cameraLeftCentroids[i].x;
				channelSet[c++].samples[i] = this->cameraLeftCentroids[i].y;
				channelSet[c++].samples[i] = this->cameraRightCentroids[i].x;
				channelSet[c++].samples[i] = this->cameraRightCentroids[i].y;

				channelSet[c++].samples[i] = this->epipolarDistance[i];
				channelSet[c++].samples[i] = this->massRatio[i];
				channelSet[c++].samples[i] = this->angleDistance[i];
			}

			return true;
		}
	}
}