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
#ifndef DIVIDENDEN_CSV_HPP_INCLUDED
#define DIVIDENDEN_CSV_HPP_INCLUDED

#include "utils/csv_reader.hpp"
#include "stocks.hpp"
#include "utils/utils.hpp"

namespace stocks_dl {

class dividenden_csv:
    public csv_reader::table_consumer
{
private:
	std::vector<dividend> m_dividends;

public:
	string_to_int_limited_converter convertobj;
	
    enum {
        dividenden_csv_date = 0,
        dividenden_csv_dividends
    };

    // table_consumer
    virtual long int GetColumnIndex(std::string column_heading);
    virtual bool AddLine(std::map<long int, std::string> row);

    std::vector<dividend> Get();
};

} // namespace stocks_dl

#endif // DIVIDENDEN_CSV_HPP_INCLUDED
