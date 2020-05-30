#pragma once

#include "MoCapLib.h"

#include "pch_OP_SyncCameras.h"

#include <filesystem>

#ifdef OP_RECORDER_EXPORTS
	#define OP_RECORDER_API DLLEXPORT
#else
	#define OP_RECORDER_API
#endif
