#pragma once

#include "MoCapLib.h"

namespace TD_MoCap {
	struct CameraParameters
	{
		Utils::ValueParameter<std::string> serialNumber
		{
			"Serial number"
			, ""
			, ""
		};

		Utils::ValueParameter<bool> limitBandwidth
		{
			"Limit bandwidth"
			, false
			, false
		};

		Utils::NumberParameter<int> maximumBandwidth
		{
			"Maximum bandwidth", "Mbit/s"
			, 1500, 1500
			, 0, 20000
			, 0, 20000
		};

		Utils::NumberParameter<float> exposure
		{
			"Exposure", "ms"
			, 10, 10
			, 0, 10000
			, 0, 100
		};

		Utils::ValueParameter<bool> preview
		{
			"Preview", false, false
		};

		Utils::SelectorParameter trigger
		{
			"Trigger"
			, {"Freerun", "Mainloop", "Manual", "Follower"}
			, "Freerun"
		};


		Utils::ParameterList list{
			&serialNumber
			, &limitBandwidth
			, &maximumBandwidth
			, &exposure
			, &preview
			, &trigger
		};
		std::set<Utils::AbstractParameter*> stale;
		std::mutex mutex;
	};
}