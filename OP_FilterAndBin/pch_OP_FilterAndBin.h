#pragma once

#include "MoCapLib.h"

#include "pch_MoCapLib.h"

#ifdef OP_FILTERANDBIN_EXPORT
	#define OP_FILTERANDBIN_API DLLEXPORT
#else
	#define OP_FILTERANDBIN_API
#endif
