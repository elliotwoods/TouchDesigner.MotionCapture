#pragma once

namespace TD_MoCap {
	namespace Utils {
		class TDMOCAP_API Table
		{
		public:
			class RowBuilder
			{
			public:
				RowBuilder(Table&);
				~RowBuilder();

				template<typename T>
				RowBuilder & operator<<(const T& cellData) {
					std::stringstream ss;
					ss << cellData;
					this->cells.push_back(ss.str());
					return *this;
				}
			private:
				Table& table;
				std::vector<std::string> cells;
			};

			Table();
			RowBuilder newRow();
			void addRow(const std::vector<std::string>&);

			void populateOutput(DAT_Output*);

		protected:
			std::vector<std::vector<std::string>> cells;
		};
	}
}