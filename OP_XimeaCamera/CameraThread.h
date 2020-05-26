#pragma once

#include "xiAPIplus/xiapiplus.h"

#include "Utils/WorkerThread.h"
#include "Utils/Parameter.h"

#include <memory>
#include "pch_OP_XimeaCamera.h"

namespace TD_MoCap {
	class OP_XIMEACAMERA_API CameraThread : public std::enable_shared_from_this<CameraThread>
	{
	public:
		CameraThread(const OP_Inputs*, const Utils::ParameterList &, Links::Output&);
		~CameraThread();

		void performInThread(std::function<void(xiAPIplus_Camera&)>, bool blocking);

		void pushToCamera(Utils::AbstractParameter*);
		void pullFromCamera(Utils::AbstractParameter*);

		void requestManualTrigger();

		Utils::Channel<Exception>& getThreadExceptionQueue();

		xiAPIplus_Camera& getCamera();
		Utils::WorkerThread& getThread();
	protected:
		void requestCapture();
		void rethrowFormattedExceptions(std::function<void()>);

		Links::Output& output;

		bool showPreviewWindow = false;
		bool joining = false;
		Utils::WorkerThread workerThread;
		xiAPIplus_Camera camera;

		const std::string windowName = "Ximea preview";
	};
}