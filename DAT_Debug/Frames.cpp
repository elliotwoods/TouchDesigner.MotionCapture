#include "pch_DAT_Debug.h"
#include "Frames.h"
#include "MoCapLib.h"

namespace TD_MoCap {
	//----------
	Utils::Table
		debugFrames()
	{
		// accumulate data
		std::map<std::string, size_t> countPerType;
		{
			const auto& allFrames = Frames::BaseFrame::getAllFrames();
			for (const auto& frame : allFrames) {
				auto typeName = frame->getTypeName();
				auto it = countPerType.find(typeName);
				if (it == countPerType.end()) {
					countPerType.emplace(typeName, 1);
				}
				else {
					it->second++;
				}
			}
		}

		// build table
		Utils::Table table;
		{
			table.newRow() << "Frame type" << "count";

			for (const auto& count : countPerType)
			{
				table.newRow() << count.first << count.second;
			}
		}

		return table;
	}
}
