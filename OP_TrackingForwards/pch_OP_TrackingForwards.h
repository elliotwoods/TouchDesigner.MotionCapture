#pragma once

#include "MoCapLib.h"

#include "pch_MoCapLib.h"

#ifdef OP_TRACKINGFORWARDS_EXPORT
	#define OP_TRACKINGFORWARDS_API DLLEXPORT
#else
	#define OP_TRACKINGFORWARDS_API
#endif
