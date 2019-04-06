/* Copyright (C) 2019 Matthias Rosenthal
 *
 * This file is part of stocks_dl.
 *
 * Stocks_dl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * Stocks_dl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with stocks_dl. If not, see <http://www.gnu.org/licenses/>
 **********************************************************************/
/**************************************************************************************
 * csv_reader
 *
 * Parser for CSV files
 * A csv file is structed like this
 * Example with separator char = ','
 * 1. line:         <column 1 heading>,<column 2 heading>
 * all other lines: <column 1 value for row x>,<column 2 value for row x>
 * 
 *************************************************************************************/

#ifndef STOCKS_DL_CSV_READER_HPP_INCLUDED
#define STOCKS_DL_CSV_READER_HPP_INCLUDED

#include <istream>
#include <map>
#include <list>
#include <string>

namespace stocks_dl {
namespace csv_reader {

class table_consumer
{
public:
    // If the same column indexes are returned multiply, only the last column that got this
    // index is matched to the index.
    virtual long int GetColumnIndex(std::string column_heading) = 0;
    virtual bool AddLine(std::map<long int, std::string> zeile) = 0; // Returning false means wrong data format
};

// A reader object is only for one-time use
class reader
{
private:
    char m_separator;
    table_consumer* m_table_consumer;

    std::list<std::string> m_column_headings;
    std::list<long int> m_columns; // Column indexes
    std::string current_column_heading;
    std::string current_value;
    long int status;
    enum {
        reader_stat_line1_column,
        reader_stat_value
    };
    unsigned int line_number;
    std::list<long int>::iterator it_column;
    std::map<long int, std::string> current_line;

public:
    reader(char separator, table_consumer* c_table_consumer);
    ~reader();

    int Parse(std::istream& instream); // 0 bei Erfolg
    std::string GetError(int fehler); // Returns error description with corresponding line number
    
    enum errors {
		CSVERR_NO_ERROR = 0,
		CSVERR_FIRST_LINE_UNCOMPLETE,
		CSVERR_EMPTY_COLUMN_HEADING,
		CSVERR_DUPLICATE_COLUMN,
		CSVERR_MORE_COLUMNS_IN_ONE_ROW, // One row has more columns than the rows before
		CSVERR_WRONG_DATA_FORMAT
	};		

private:
    void InitParserData();
    int ParseChar(char zeichen);
};

} // namespace csv_reader
} // namespace stocks_dl

#endif // STOCKS_DL_CSV_READER_HPP_INCLUDED
