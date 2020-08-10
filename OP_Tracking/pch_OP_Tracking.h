#pragma once

#include "MoCapLib.h"

#include "pch_MoCapLib.h"

#ifdef OP_TRACKING_EXPORT
	#define OP_TRACKING_API DLLEXPORT
#else
	#define OP_TRACKING_API
#endif
