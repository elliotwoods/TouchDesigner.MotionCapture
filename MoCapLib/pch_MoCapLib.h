#pragma once

#include "CPlusPlus_Common.h"
#include "DAT_CPlusPlusBase.h"
#include "TOP_CPlusPlusBase.h"
#include "GL_Extensions.h"

#include <nlohmann/json.hpp>

#include <opencv2/opencv.hpp>

#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <functional>
#include <fstream>
#include <filesystem>
#include <vector>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#ifdef TDMOCAP_EXPORTS
	#define TDMOCAP_API DLLEXPORT
#else
	#define TDMOCAP_API
#endif