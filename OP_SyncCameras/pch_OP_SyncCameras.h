#pragma once

#include "MoCapLib.h"

#include "CameraThread.h"

#include <filesystem>

#ifdef OP_SYNCCAMERAS_EXPORT
	#define OP_SYNCCAMERAS_API DLLEXPORT
#else
	#define OP_SYNCCAMERAS_API
#endif
