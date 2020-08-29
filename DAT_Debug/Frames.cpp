#include "pch_DAT_Debug.h"
#include "Frames.h"
#include "MoCapLib.h"

namespace TD_MoCap {
	//----------
	Utils::Table
		debugFrames()
	{
		// accumulate data

		// build table
		Utils::Table table;
		{
			table.newRow() << "Frame type" << "count";
			const auto countPerType = Frames::BaseFrame::getCountPerType();

			for (const auto& count : countPerType)
			{
				table.newRow() << count.first << count.second;
			}
		}

		return table;
	}
}
