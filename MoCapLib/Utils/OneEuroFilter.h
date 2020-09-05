#pragma once

#include <glm/glm.hpp>

// adapted from https://cristal.univ-lille.fr/~casiez/1euro/OneEuroFilter.cc

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API LowPassFilter {
			float y, alpha, s;
			bool initialized;

			void setAlpha(float);

		public:
			void init(float alpha, float initval = 0.0);

			float filter(float value);
			float filterWithAlpha(float value, float alpha);
			bool hasLastRawValue() const;
			float lastRawValue() const;

		};

		class TDMOCAP_API OneEuroFilter {
		public:
			struct Params {
				float frequency;
				float minCutOff;
				float beta;
				float dCutOff;
			};

			OneEuroFilter(const Params &);
			OneEuroFilter(float samplingFrequency
				, float minCutOff = 1.0f
				, float beta = 0.0f
				, float dCutOff = 1.0f);
			~OneEuroFilter();

			float filter(float value);
		private:
			float samplingFrequency;
			float mincutoff;
			float beta;
			float dcutoff;
			LowPassFilter x;
			LowPassFilter dx;

			float alpha(float cutoff) const;
			void setSamplingFrequency(float);
			void setMinCutoff(float);
			void setBeta(float);
			void setDerivativeCutoff(float);
		};
	}
}
