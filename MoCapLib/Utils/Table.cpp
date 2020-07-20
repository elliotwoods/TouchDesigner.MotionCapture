#include "pch_MoCapLib.h"
#include "Table.h"

namespace TD_MoCap {
	namespace Utils {
		//----------
		Table::RowBuilder::RowBuilder(Table& table)
			: table(table)
		{
		}

		//----------
		Table::RowBuilder::RowBuilder(RowBuilder&& other)
			: table(other.table)
		{
			other.isSuppressed = true;

			this->cells = std::move(other.cells);
		}

		//----------
		Table::RowBuilder::~RowBuilder()
		{
			if (!this->isSuppressed) {
				table.addRow(this->cells);
			}
		}

		//----------
		Table::Table() {

		}

		//----------
		Table::RowBuilder
			Table::newRow()
		{
			return RowBuilder(*this);
		}

		//----------
		void
			Table::addRow(const std::vector<std::string>& cells)
		{
			this->cells.push_back(cells);
		}

		//----------
		void
			Table::clear()
		{
			this->cells.clear();
		}

		//----------
		void
			Table::populateOutput(DAT_Output* output)
		{
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

		//----------
		void
			Table::getInfoSize(OP_InfoDATSize* infoSize)
		{
			uint32_t maxCols = 0;
			for (const auto& row : this->cells) {
				if (row.size() > maxCols) {
					maxCols = row.size();
				}
			}

			infoSize->cols = maxCols;
			infoSize->rows = this->cells.size();
			infoSize->byColumn = true;
		}

		//----------
		void
			Table::populateInfoEntries(OP_InfoDATEntries* entries, int32_t index)
		{
			for (int iRow = 0; iRow < this->cells.size(); iRow++) {
				auto& row = this->cells[iRow];
				entries[0].values[iRow][0].setString(row[index].c_str());
			}
		}
	}
}