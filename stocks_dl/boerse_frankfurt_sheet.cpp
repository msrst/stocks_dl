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

#include "boerse_frankfurt_sheet.hpp"
#include <iostream>

namespace stocks_dl {
namespace boerse_frankfurt {

void balance_sheet::Print()
{
	std::cout << year << ", currency: " << currency << std::endl;
	for(unsigned int i1 = 0; i1 < BALANCE_SHEET_FIELD_COUNT; i1++) {
		if(data[i1]) {
			if(printed_mio[i1]) {
				std::cout << field_names[i1] + " in Mio."
					<< std::string(field_names_max_len + 1 - field_names[i1].length(), ' ')
					<< data[i1] / 1000000 << std::endl;
			}
			else {
				std::cout << field_names[i1]
					<< std::string(field_names_max_len + 1 + 8 - field_names[i1].length(), ' ')
					<< data[i1] << std::endl;
			}
		}
	}
}

} // namespace boerse_frankfurt
} // namespace stocks_dl
