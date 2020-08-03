#include "pch_MoCapLib.h"
#include "FrameRateCounter.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		FrameRateCounter::FrameRateCounter(uint16_t windowSize)
		{
			this->data = new std::chrono::high_resolution_clock::time_point[windowSize];
			this->windowSize = windowSize;
		}

		//----------
		FrameRateCounter::~FrameRateCounter()
		{
			delete[] this->data;
		}

		//----------
		void
			FrameRateCounter::clear()
		{
			this->position = 0;
			this->firstRun = 0;
		}

		//----------
		void
			FrameRateCounter::tick()
		{
			this->position++;
			this->position %= this->windowSize;
			this->data[this->position] = std::chrono::high_resolution_clock::now();

			if (this->firstRun < this->windowSize) {
				this->firstRun++;
			}
		}

		//----------
		float
			FrameRateCounter::getFPS() const
		{
			if (this->firstRun < this->windowSize) {
				if (this->firstRun < 2) {
					// not enough data yet
					return 0.0f;
				}
				else {
					// buffer is only partially full
					auto dt = this->data[this->position - 1] - this->data[0];
					return (1000.0f * (this->position - 1)) / std::chrono::duration<float, std::milli>(dt).count();
				}
			}
			else {
				// buffer is full
				auto windowEnd = this->position;
				auto windowBegin = (this->position + 1) % this->windowSize;

				auto dt = this->data[windowEnd] - this->data[windowBegin];
				return (1000.0f * (this->windowSize - 1)) / std::chrono::duration<float, std::milli>(dt).count();
			}
		}

		//----------
		float
			FrameRateCounter::getFPSUnfiltered() const
		{
			if (this->firstRun < 2) {
				// not enough data yet
				return 0.0f;
			}
			else {
				auto currentPosition = this->position - 1;
				auto previousPosition = this->position - 2;
				if (currentPosition < 0) {
					currentPosition += windowSize;
				}
				if (previousPosition < 0) {
					previousPosition += windowSize;
				}
				auto dt = this->data[currentPosition] - this->data[previousPosition];
				return (1000.0f) / std::chrono::duration<float, std::milli>(dt).count();
			}
		}
	}
}