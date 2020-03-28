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

#ifndef BOERSE_FRANKFURT_PARSER_HPP_INCLUDED
#define BOERSE_FRANKFURT_PARSER_HPP_INCLUDED

#include <map>
#include "utils/logger.hpp"
#include "curlwrapper.hpp"
#include "boerse_frankfurt_sheet.hpp"

namespace stocks_dl {
namespace boerse_frankfurt {

class bf_converter
{
private:
	logger_base_ptr m_logger;
	std::vector<std::string> bf_price_headings;
	std::string bf_kurs_heading_heading;
	std::vector<std::string> bf_dividend_headings;
	std::string bf_dividend_heading_heading;
	
public:
	balance_sheet_field currency_balance_sheet_field;
	std::vector<balance_sheet_field> balance_sheet_fields;
	std::map<std::string, unsigned int> bf_to_num;
	bf_converter(logger_base_ptr c_logger);
	
	// zero when successful.
	int BalanceSheetFromJson(long int stock_id, const std::string &json, std::vector<balance_sheet> &balance_sheets); // Appends the balance sheets. In
			// the appended balance sheets, the stock_id of the balance sheets is set to stock_id.
	int PricesFromJson(const std::string &json, std::vector<daily_price> &prices); // Overwrites prices.
	int DividendsFromJson(const std::string &isin_check, const std::string &json, std::vector<dividend> &dividends); // Overwrites dividends. Isin is only for printing.
	
	// true when sucessful
	bool GetBFName_ViaSearch(const std::string &isin, mcurlwrapper &mcurl, std::string &bfname);
	bool GetBFName_ViaStandard(const std::string &simple_name, mcurlwrapper &mcurl, std::string &bfname); // Broken since second half of 2019
};

} // namespace boerse_frankfurt
} // namespace stocks_dl

#endif // BOERSE_FRANKFURT_PARSER_HPP_INCLUDED
