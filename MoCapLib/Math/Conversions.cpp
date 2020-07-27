#include "pch_MoCapLib.h"
#include "Conversions.h"

#define MATCHED_TYPE_OF_CV_BODY(X, Y) \
Y & toCv(X & x) { \
return * (Y *) & x; \
} \
const Y & toCv(const X & x) { \
return * (const Y *) & x; \
} \
X & toGlm(Y & y) { \
return * (X *) & y; \
} \
const X & toGlm(const Y & y) { \
return * (const X *) & y; \
} 

#define MATCHED_TYPE_VECTOR_BODY(X, Y) \
MATCHED_TYPE_OF_CV_BODY(X, Y) \
MATCHED_TYPE_OF_CV_BODY(std::vector<X>, std::vector<Y>) \
MATCHED_TYPE_OF_CV_BODY(std::vector<std::vector<X>>, std::vector<std::vector<Y>>)

namespace TD_MoCap {
	MATCHED_TYPE_VECTOR_HEADER(glm::vec2, cv::Point2f);
	MATCHED_TYPE_VECTOR_HEADER(glm::vec3, cv::Point3f);
}