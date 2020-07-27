#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

#define MATCHED_TYPE_OF_CV_HEADER(X, Y) \
Y & toCv(X &);\
const Y & toCv(const X &);\
X & toGlm(Y &);\
const X & toOf(const Y &);

#define MATCHED_TYPE_VECTOR_HEADER(X, Y) \
MATCHED_TYPE_OF_CV_HEADER(X, Y) \
MATCHED_TYPE_OF_CV_HEADER(std::vector<X>, std::vector<Y>) \
MATCHED_TYPE_OF_CV_HEADER(std::vector<std::vector<X>>, std::vector<std::vector<Y>>)

namespace TD_MoCap {
	MATCHED_TYPE_VECTOR_HEADER(glm::vec2, cv::Point2f);
	MATCHED_TYPE_VECTOR_HEADER(glm::vec3, cv::Point3f);

}