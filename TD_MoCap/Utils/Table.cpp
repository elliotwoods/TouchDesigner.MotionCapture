#include "pch_TD_MoCap.h"
#include "Table.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		Table::RowBuilder::RowBuilder(Table& table)
			: table(table) {

		}

		//----------
		Table::RowBuilder::~RowBuilder() {
			table.addRow(this->cells);
		}

		//----------
		Table::Table() {

		}

		//----------
		Table::RowBuilder
		Table::newRow() {
			return RowBuilder(*this);
		}

		//----------
		void
		Table::addRow(const std::vector<std::string>& cells) {
			this->cells.push_back(cells);
		}

		//----------
		void
		Table::populateOutput(DAT_Output* output) {
			output->setOutputDataType(DAT_OutDataType::Table);
			
			//find column count;
			uint32_t maxCols = 0;
			for (const auto& row : this->cells) {
				if (row.size() > maxCols) {
					maxCols = row.size();
				}
			}

			output->setTableSize(this->cells.size(), maxCols);
			
			for (int row = 0; row < this->cells.size(); row++) {
				for (int col = 0; col < this->cells[row].size(); col++) {
					output->setCellString(row, col, this->cells[row][col].c_str());
				}
			}
		}
	}
}