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

#include <iostream>
#include <limits>

#include "dividends.hpp"

namespace stocks_dl {

long int dividenden_csv::GetColumnIndex(std::string column_heading)
{
    if(column_heading == "Date") {
        return dividenden_csv_date;
    }
    else if(column_heading == "Dividends") {
        return dividenden_csv_dividends;
    }
    else {
        return -1;
    }
}
bool dividenden_csv::AddLine(std::map<long int, std::string> row)
{
    std::map<long int, std::string>::iterator it_datum, it_dividende;
    it_datum = row.find(dividenden_csv_date);
    it_dividende = row.find(dividenden_csv_dividends);
    if((it_datum == row.end()) || (it_dividende == row.end())) {
        return false;
    }
    else {
		dividend neue_dividende;
		neue_dividende.date = TimeDayFromString(it_datum->second);
		neue_dividende.dividend = convertobj.ConvertToDouble_Limited(it_dividende->second, 0, std::numeric_limits<double>::max());
        m_dividends.push_back(neue_dividende);
        return true;
    }
}

std::vector<dividend> dividenden_csv::Get()
{
    return m_dividends;
}

} // namespace stocks_dl
