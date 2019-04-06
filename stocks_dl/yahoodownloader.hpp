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
#ifndef YAHOODOWNLOADER_HPP_INCLUDED
#define YAHOODOWNLOADER_HPP_INCLUDED

#include "curlwrapper.hpp"
#include "stocks.hpp"

namespace stocks_dl {
	
class yahoodownloader
{
private:
	mcurlwrapper m_curlwrapper;
	logger_base_ptr m_logger;
	bool b_ok;
	
public:
	yahoodownloader(logger_base_ptr c_logger):
			m_curlwrapper(c_logger) {
		b_ok = false;
		m_logger = c_logger;
		if(m_curlwrapper.Init() == 0) {
			if(m_curlwrapper.EnableCookieEngine() == 0) {
				b_ok = true;
			}
		}
	}
	bool IsOk() { // returns whether the initalisation of libcurl was successful
		return b_ok;
	}
	// The symbol passed to yahoo musts be calculated outside and passed via ysymbol.
	// Download(.) tests whether c_share.name can be found in the html document at the web page at 
	// finance.yahoo.com corresponding to the share. If not, the function aborts and returns a
	// value unequal to zero.
	// When successful, returns zero and overwrites dividends with the result.
	int Download(const stock_share &c_share, std::string ysymbol, time_day trading_day_min, time_day trading_day_max, std::vector<dividend> &dividends);
};

} // namespace stocks_dl

#endif // YAHOODOWNLOADER_HPP_INCLUDED
