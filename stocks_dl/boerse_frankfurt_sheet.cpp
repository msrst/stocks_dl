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