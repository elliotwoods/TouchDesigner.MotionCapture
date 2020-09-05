#include "pch_MoCapLib.h"
#include "OneEuroFilter.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		void
			LowPassFilter::setAlpha(float alpha)
		{
			if (alpha <= 0.0) {
				alpha = 0.0f;
			}
			else if (alpha > 1.0) {
				alpha = 1.0f;
			}
			this->alpha = alpha;
		}

		//----------
		void
		LowPassFilter::init(float alpha, float initval)
		{
			this->y = this->s = initval;
			this->setAlpha(alpha);
			this->initialized = false;
		}

		//----------
		float
			LowPassFilter::filter(float value)
		{
			float result;
			if (initialized)
				result = this->alpha * value + (1.0 - this->alpha) * this->s;
			else {
				result = value;
				this->initialized = true;
			}
			this->y = value;
			this->s = result;
			return result;
		}

		//----------
		float
			LowPassFilter::filterWithAlpha(float value, float alpha)
		{
			this->setAlpha(alpha);
			return this->filter(value);
		}

		//----------
		bool
			LowPassFilter::hasLastRawValue() const
		{
			return this->initialized;
		}

		//----------
		float
			LowPassFilter::lastRawValue() const
		{
			return this->y;
		}


#define M_PI 3.14159265358979323846264338327950288

		//----------
		float
			OneEuroFilter::alpha(float cutoff) const
		{
			float te = 1.0 / this->samplingFrequency;
			float tau = 1.0 / (2 * M_PI * cutoff);
			return 1.0f / (1.0f + tau / te);
		}

		//----------
		void
			OneEuroFilter::setSamplingFrequency(float samplingFrequency)
		{
			if (samplingFrequency <= 0.0f) {
				samplingFrequency = 1.0f;
			}
			this->samplingFrequency = samplingFrequency;
		}

		//----------
		void
			OneEuroFilter::setMinCutoff(float mc)
		{
			if (mc <= 0 - .0f) {
				mc = 1.0f;
			}
			this->mincutoff = mc;
		}

		//----------
		void
			OneEuroFilter::setBeta(float beta)
		{
			this->beta = beta;
		}

		//----------
		void
			OneEuroFilter::setDerivativeCutoff(float dc)
		{
			if (dc <= 0.0f) {
				dc = 1.0f;
			}
			this->dcutoff = dc;
		}

		//----------
		OneEuroFilter::OneEuroFilter(const Params & params)
		: OneEuroFilter(params.frequency
			, params.minCutOff
			, params.beta
			, params.dCutOff)
		{
		}

		//----------
		OneEuroFilter::OneEuroFilter(float samplingFrequency
			, float minCutOff
			, float beta
			, float dCutOff)
		{
			this->setSamplingFrequency(samplingFrequency);
			this->setMinCutoff(minCutOff);
			this->setBeta(beta);
			this->setDerivativeCutoff(dCutOff);
			this->x.init(this->alpha(mincutoff));
			this->dx.init(this->alpha(dcutoff));
		}

		//----------
		OneEuroFilter::~OneEuroFilter()
		{

		}

		//----------
		float
		OneEuroFilter::filter(float value)
		{
			// estimate the current variation per second 
			float dvalue = this->x.hasLastRawValue() ? (value - this->x.lastRawValue()) * this->samplingFrequency : 0.0f; // FIXME: 0.0 or value?
			float edvalue = this->dx.filterWithAlpha(dvalue, this->alpha(this->dcutoff));

			// use it to update the cutoff frequency
			float cutoff = this->mincutoff + this->beta * fabs(edvalue);

			// filter the given value
			return this->x.filterWithAlpha(value, this->alpha(cutoff));
		}
	}
}