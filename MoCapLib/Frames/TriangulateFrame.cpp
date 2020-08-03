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
				"camera1_ray_s_x",
				"camera1_ray_s_y",
				"camera1_ray_s_z",
				"camera1_ray_t_x",
				"camera1_ray_t_y",
				"camera1_ray_t_z",

				"camera2_ray_s_x",
				"camera2_ray_s_y",
				"camera2_ray_s_z",
				"camera2_ray_t_x",
				"camera2_ray_t_y",
				"camera2_ray_t_z",

				"intersect_ray_s_x",
				"intersect_ray_s_y",
				"intersect_ray_s_z",
				"intersect_ray_t_x",
				"intersect_ray_t_y",
				"intersect_ray_t_z"
				});

			auto size = this->testData.raysLeft.size();
			channelSet.setSampleCount(size);

			for (size_t i = 0; i < size; i++) {
				int c = 0;

				channelSet[c++].samples[i] = this->testData.raysLeft[i].s.x;
				channelSet[c++].samples[i] = this->testData.raysLeft[i].s.y;
				channelSet[c++].samples[i] = this->testData.raysLeft[i].s.z;
				channelSet[c++].samples[i] = this->testData.raysLeft[i].t.x;
				channelSet[c++].samples[i] = this->testData.raysLeft[i].t.y;
				channelSet[c++].samples[i] = this->testData.raysLeft[i].t.z;

				channelSet[c++].samples[i] = this->testData.raysRight[i].s.x;
				channelSet[c++].samples[i] = this->testData.raysRight[i].s.y;
				channelSet[c++].samples[i] = this->testData.raysRight[i].s.z;
				channelSet[c++].samples[i] = this->testData.raysRight[i].t.x;
				channelSet[c++].samples[i] = this->testData.raysRight[i].t.y;
				channelSet[c++].samples[i] = this->testData.raysRight[i].t.z;


				channelSet[c++].samples[i] = this->testData.intersections[i].s.x;
				channelSet[c++].samples[i] = this->testData.intersections[i].s.y;
				channelSet[c++].samples[i] = this->testData.intersections[i].s.z;
				channelSet[c++].samples[i] = this->testData.intersections[i].t.x;
				channelSet[c++].samples[i] = this->testData.intersections[i].t.y;
				channelSet[c++].samples[i] = this->testData.intersections[i].t.z;
			}

			return true;
		}
	}
}