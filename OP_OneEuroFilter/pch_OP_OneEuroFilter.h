#pragma once

#include "MoCapLib.h"

#include "pch_MoCapLib.h"

#ifdef OP_ONEEUROFILTER_EXPORT
	#define OP_ONEEUROFILTER_API DLLEXPORT
#else
	#define OP_ONEEUROFILTER_API
#endif
