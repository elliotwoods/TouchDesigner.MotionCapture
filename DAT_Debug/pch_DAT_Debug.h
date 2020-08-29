#pragma once

#include "MoCapLib.h"

#ifdef DAT_DEBUG_EXPORTS
	#define DAT_DEBUG_API DLLEXPORT
#else
	#define DAT_DEBUG_API
#endif