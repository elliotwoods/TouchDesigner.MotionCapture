#pragma once

#include "xiAPIplus/xiapiplus.h"

#include "Utils/WorkerThread.h"
#include "Utils/Parameter.h"

namespace TD_MoCap {
	class CameraThread {
	public:
		CameraThread(const OP_Inputs*, const Utils::ParameterList &, Links::Output&);
		~CameraThread();

		void performInThread(std::function<void(xiAPIplus_Camera&)>, bool blocking);

		void pushToCamera(Utils::AbstractParameter*);
		void pullFromCamera(Utils::AbstractParameter*);

		Utils::Channel<Exception> errorsFromThread;
	protected:
		void requestCapture();
		void runAndFormatExceptions(std::function<void()>);

		Links::Output& output;

		bool joining = false;
		Utils::WorkerThread workerThread;
		xiAPIplus_Camera camera;
	};
}