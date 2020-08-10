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

		struct Cube {
			std::vector<glm::vec3> vertices
			{
				{-1, -1, -1}
				, {1, -1, -1}
				, {1, 1, -1}
				, {-1, 1, -1}
				, {-1, -1, 1}
				, {1, -1, 1}
				, {1, 1, 1}
				, {-1, 1, 1}
			};

			std::vector<int32_t> lineIndices
			{
				0, 1, 2, 3, 0,

				4, 5, 6, 7, 4,

				0, 4,
				1, 5,
				2, 6,
				3, 7
			};

			std::vector<int32_t> lineSize
			{
				5,
				5,

				2,
				2,
				2,
				2,
			};

			std::vector<int32_t> primitiveIndices
			{
				0, 1,
				2, 3, 4, 5
			};

			void offsetPointIndices(int32_t & offset)
			{
				for (auto& index : this->lineIndices) {
					index += offset;
				}
				offset = this->lineIndices.back() + 1;
			}

			void offsetPrimitiveIndices(int32_t & offset)
			{
				for (auto& index : this->primitiveIndices) {
					index += offset;
				}
				offset = this->primitiveIndices.back() + 1;
			}

			void transform(const glm::mat4& transform)
			{
				for (auto& vertex : this->vertices) {
					auto pos4 = transform * glm::vec4(vertex, 1.0f);
					pos4 /= pos4.w;
					vertex = (glm::vec3) pos4;
				}
			}

			void addToSOP(SOP_Output* output, const char * name, int32_t & pointIndex, int32_t & primitiveIndex) {
				
				output->addPoints((Position*)this->vertices.data(), this->vertices.size());
				this->offsetPointIndices(pointIndex);

				output->addLines(this->lineIndices.data(), this->lineSize.data(), this->lineSize.size());
				this->offsetPrimitiveIndices(primitiveIndex);

				output->addGroup(SOP_GroupType::Primitive, name);
				for (const auto& primitiveIndex : this->primitiveIndices) {
					output->addPrimToGroup(primitiveIndex, name);
				}
			}
		};

		//----------
		bool
			TriangulateFrame::getPreviewSOP(SOP_Output* output) const
		{
			int32_t pointIndex = 0;
			int32_t primitiveIndex = 0;

			{
				Cube camera;
				camera.transform(this->cameraLeft.getInverseViewProjection());
				camera.addToSOP(output, "Camera Left", pointIndex, primitiveIndex);
			}

			{
				Cube camera;
				camera.transform(this->cameraRight.getInverseViewProjection());
				camera.addToSOP(output, "Camera Right", pointIndex, primitiveIndex);
			}

			output->addPoints((Position*)this->worldPoints.data(), this->worldPoints.size());
			return true;
		}
	}
}