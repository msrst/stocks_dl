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

#ifndef BOERSE_FRANKFURT_HPP_INCLUDED
#define BOERSE_FRANKFURT_HPP_INCLUDED

#include <map>
#include "utils/logger.hpp"
#include "curlwrapper.hpp"
#include "stocks.hpp"
#include "boerse_frankfurt_parser.hpp"

namespace stocks_dl {
namespace boerse_frankfurt {

// Special downloader for prices (some tricks are needed to compute necessary cookies)
class price_downloader
{
private:
	mcurlwrapper mcurl;
	curl_slist* slist_reset = NULL;
	bf_converter *m_bf_converter;
	logger_base_ptr m_logger;
	
public:
	price_downloader(bf_converter *c_bf_converter, logger_base_ptr c_logger):
			mcurl(c_logger) {
		slist_reset = curl_slist_append(NULL, "__atcrv:");
		curl_slist_append(slist_reset, "__ath:");
		curl_slist_append(slist_reset, "__atts:");
		
		m_bf_converter = c_bf_converter;
		m_logger = c_logger;
	}
	~price_downloader() {
		curl_slist_free_all(slist_reset);
	}
	int Init() {
		if(mcurl.Init()) {
			return 1;
		}
		return 0;
	}
	
	// Returns: 0 = success
	int Download(const std::string &isin, time_day date_min, time_day date_max, 
		std::vector<daily_price> &prices, std::string stock_exchange = FRANKFURT_STOCK_EXCHANGE);
};

// Downloader for balance sheet data, wkn and symbol_name (the symbol name 
// is something like JNJ for Johnson&Johnson and is used, p. e., by finance.yahoo.com).
// Also for dividends
class share_downloader
{
private:
	mcurlwrapper mcurl;
	bf_converter *m_bf_converter;
	logger_base_ptr m_logger;
	
public:
	share_downloader(bf_converter *c_bf_converter, logger_base_ptr c_logger):
			mcurl(c_logger) {
		m_bf_converter = c_bf_converter;
		m_logger = c_logger;
	}
	int Init() {
		if(mcurl.Init()) {
			return 1;
		}
		return 0;
	}
	
	// Downloads balance sheet data, wkn and stock exchange symbol.
	// Returns: 0 = success
	int Download(long int share_id, const std::string &isin, std::vector<balance_sheet> &balance_sheets, std::string &wkn, std::string &symbol_name, int limit = 50, std::string stock_exchange = FRANKFURT_STOCK_EXCHANGE);
	// Normally, boerse_frankfurt.de delivers only the dividends of the last seven years and not for US shares.
	int DownloadDividends(const std::string &isin, std::vector<dividend> &dividends, int limit = 50, std::string stock_exchange = FRANKFURT_STOCK_EXCHANGE);
};

} // namespace boerse_frankfurt

bool FindAJAXString(const std::string &sitedata, const std::string &str_begin, std::string &ajax_str, logger_base_ptr);

} // namespace stocks_dl

#endif // BOERSE_FRANKFURT_HPP_INCLUDED
