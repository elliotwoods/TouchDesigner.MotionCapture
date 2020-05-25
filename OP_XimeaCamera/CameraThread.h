#pragma once

#include "xiAPIplus/xiapiplus.h"

#include "Utils/WorkerThread.h"
#include "Utils/Parameter.h"

namespace TD_MoCap {
	class CameraThread {
	public:
		struct Parameter {
			std::string name;
			std::string unit;
			uint64_t value;
			uint64_t defaultValue;
			uint64_t min;
			uint64_t max;
		};

		CameraThread(const OP_Inputs*, const Utils::ParameterList &, Links::Output&);
		~CameraThread();

		void performInThread(std::function<void(xiAPIplus_Camera&)>, bool blocking);

		void pushToCamera(Utils::AbstractParameter*);
		void pullFromCamera(Utils::AbstractParameter*);
	protected:
		void requestCapture();
		void runAndFormatExceptions(std::function<void()>);

		Links::Output& output;

		bool joining = false;
		Utils::WorkerThread workerThread;
		xiAPIplus_Camera camera;
	};
}