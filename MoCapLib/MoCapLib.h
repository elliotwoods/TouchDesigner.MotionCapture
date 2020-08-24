#pragma once

#include "pch_MoCapLib.h"

#include "Links/Input.h"
#include "Links/Output.h"
#include "Links/OutputsRegister.h"

#include "Utils/ThreadChannel.h"
#include "Utils/ChannelSet.h"
#include "Utils/Event.h"
#include "Utils/FrameRateCounter.h"
#include "Utils/Table.h"
#include "Utils/WorkerThread.h"
#include "Utils/WorkerGroup.h"
#include "Utils/Parameter.h"
#include "Utils/Parameters.h"
#include "Utils/OpticalFlow.h"

#include "Math/Camera.h"
#include "Math/Ray.h"
#include "Math/Conversions.h"

#include "Frames/BaseFrame.h"
#include "Frames/XimeaCameraFrame.h"
#include "Frames/SynchronisedFrame.h"
#include "Frames/CentroidsFrame.h"
#include "Frames/TriangulateFrame.h"
#include "Frames/TrackingFrame.h"
#include "Frames/FilterAndBinFrame.h"

#include "Exception.h"
#include "ImageFormat.h"
#include "Threaded_OP.h"
