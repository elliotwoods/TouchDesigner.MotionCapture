#pragma once
#include <glm/glm.hpp>

namespace TD_MoCap {
	namespace Math {
		class TDMOCAP_API Ray {
		public:
			Ray();
			Ray(const glm::vec3 & s, const glm::vec3 & t);
			Ray intersect(const Ray& other) const;
			glm::vec3 getMiddle() const;
			float getLength() const;

			glm::vec3 s;
			glm::vec3 t;
		};
	}
}