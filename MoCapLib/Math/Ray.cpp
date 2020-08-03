#include "pch_MoCapLib.h"
#include "Ray.h"

namespace TD_MoCap {
	namespace Math {
		//----------
		Ray::Ray()
		{

		}

		//----------
		Ray::Ray(const glm::vec3& s, const glm::vec3& t)
		{
			this->s = s;
			this->t = t;
		}

		//----------
		Ray
			Ray::intersect(const Ray& other) const
		{
			Ray intersectRay;

			const glm::vec3 p1(this->s), p2(this->s + this->t), p3(other.s), p4(other.s + other.t);
			const float EPS(1.0E-15);

			glm::vec3 p13, p43, p21;
			float d1343, d4321, d1321, d4343, d2121;
			float numer, denom;

			p13.x = p1.x - p3.x;
			p13.y = p1.y - p3.y;
			p13.z = p1.z - p3.z;
			p43.x = p4.x - p3.x;
			p43.y = p4.y - p3.y;
			p43.z = p4.z - p3.z;
			if (fabs(p43.x) < EPS && fabs(p43.y) < EPS && fabs(p43.z) < EPS)
				return(Ray());
			p21.x = p2.x - p1.x;
			p21.y = p2.y - p1.y;
			p21.z = p2.z - p1.z;
			if (fabs(p21.x) < EPS && fabs(p21.y) < EPS && fabs(p21.z) < EPS)
				return(Ray());

			d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
			d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
			d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
			d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
			d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

			denom = d2121 * d4343 - d4321 * d4321;
			if (fabs(denom) < EPS)
				return(Ray());
			numer = d1343 * d4321 - d1321 * d4343;

			float ma = numer / denom;
			float mb = (d1343 + d4321 * (ma)) / d4343;

			glm::vec3 s;
			glm::vec3 t;

			s.x = p1.x + ma * p21.x;
			s.y = p1.y + ma * p21.y;
			s.z = p1.z + ma * p21.z;
			t.x = p3.x + mb * p43.x;
			t.y = p3.y + mb * p43.y;
			t.z = p3.z + mb * p43.z;
			t = t - s;

			return Ray(s, t);
		}

		//----------
		glm::vec3
			Ray::getMiddle() const
		{
			return this->s + this->t * 0.5f;
		}

		//----------
		float
			Ray::getLength() const
		{
			return glm::length(this->t);
		}

	}
}