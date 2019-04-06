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
#include "utils/tnull.hpp"
#include "utils/logger.hpp"
#include "table_parser.hpp"
#include "decl.hpp"
#include "curlwrapper.hpp"

namespace stocks_dl {
namespace boerse_frankfurt {

const int BALANCE_SHEET_FIELD_COUNT = 27;
const std::string FRANKFURT_STOCK_EXCHANGE = "FSE";
const std::string XETRA = "ETR";
	
// This fields are used for balance_sheet.data, p. e. my_balance_sheet.data[earning_per_share]
// equals to the earning per share.
// The fields also correspond to those of the english version of boerse-frankfurt.de:
// p. e. http://en.boerse-frankfurt.de/stock/keydata/Siemens-share/FSE#Key%20Data
enum fields {
	bffield_sales_in_mio = 0,
	bffield_dividend,
	bffield_dividend_yield, // percentage (100 equals to 100% dividend yield)
	bffield_earning_per_share,
	bffield_per, // price-earnings ratio
	bffield_ebit_in_mio,
	bffield_ebitda_in_mio,
	bffield_net_profit,
	bffield_net_profit_clean_in_mio,
	bffield_ptp, // pre-tax profit
	bffield_ptp_reported, // adjusted pre-tax profit
	bffield_eps_clean, // earnings per share
	bffield_eps_reported, // adjusted earnings per share
	bffield_gross_income,
	bffield_cashflow_investing_in_mio,
	bffield_cashflow_operations_in_mio,
	bffield_cashflow_financing_in_mio,
	bffield_cashflow_per_share,
	bffield_free_cashflow_in_mio,
	bffield_free_cashflow_per_share,
	bffield_book_value_per_share,
	bffield_net_debt_in_mio,
	bffield_research_expenses_in_mio,
	bffield_capital_expenses,
	bffield_sales_expenses,
	bffield_equity_capital_in_mio,
	bffield_total_assetts_in_mio
};
const std::string field_names[BALANCE_SHEET_FIELD_COUNT] = {
	"Sales in Mio.",
	"Dividend",
	"Dividend yield %",
	"Earnings per share",
	"PER",
	"EBIT in Mio.",
	"EBITDA in Mio.",
	"Net profit",
	"net profit clean in Mio.",
	"PTP",
	"PTP reported",
	"EPS clean",
	"EPS reported",
	"Gross income",
	"Cashflow (Investing) in Mio.",
	"Cashflow (Operations) in Mio.",
	"Cashflow (Financing) in Mio.",
	"Cashflow/share",
	"Free Cashflow in Mio.",
	"Free Cashflow per share",
	"Book value per share",
	"Net debt in Mio.",
	"Research expenses in Mio.",
	"Capital expenses",
	"Sales expenses",
	"Equity capital in Mio.",
	"Total assetts in Mio."
};
const size_t field_names_max_len = 29;

struct balance_sheet_field
{
	std::string bf_name;
	bool is_percentage;
	balance_sheet_field(std::string bf, bool is_percentage = false):
		bf_name(bf), is_percentage(is_percentage) {}
};

/* balance sheet
 * example: my_balance_sheet.data[bffield_book_value_per_share] evaluates
 * to the Value per share in year my_balance_sheet.year.
 */
struct balance_sheet
{
	long int stock_id; // freely usable, p. e. in databases
	int year;
	std::string currency; // p. e. "EUR"
	tnull_double_adv data[BALANCE_SHEET_FIELD_COUNT];
	
	void Print();
};

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
	int BalanceSheetFromTable(long int stock_id, const html_table &table, std::vector<balance_sheet> &balance_sheets); // Appends the balance sheets. In
			// the appended balance sheets, the stock_id of the balance sheets is set to stock_id.
	int PricesFromTable(const html_table &table, std::vector<daily_price> &prices); // Overwrites prices.
	int DividendsFromTable(std::string isin, const html_table &table, std::vector<dividend> &dividends); // Overwrites dividends. Isin is only for printing.
	
	// true when sucessful
	bool GetBFName_ViaSearch(const std::string &isin, mcurlwrapper &mcurl, std::string &bfname);
	bool GetBFName_ViaStandard(const std::string &simple_name, mcurlwrapper &mcurl, std::string &bfname);
};

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
	int Download(const std::string &isin, std::string date_span, std::vector<daily_price> &prices, std::string stock_exchange = FRANKFURT_STOCK_EXCHANGE);
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
	int Download(long int share_id, const std::string &isin, std::vector<balance_sheet> &balance_sheets, std::string &wkn, std::string &symbol_name, std::string stock_exchange = FRANKFURT_STOCK_EXCHANGE);
	// Normally, boerse_frankfurt.de delivers only the dividends of the last seven years and not for US shares.
	int DownloadDividends(const std::string &isin, std::vector<dividend> &dividends, std::string stock_exchange = FRANKFURT_STOCK_EXCHANGE);
};

} // namespace boerse_frankfurt

bool FindAJAXString(const std::string &sitedata, const std::string &str_begin, std::string &ajax_str, logger_base_ptr);

} // namespace stocks_dl

#endif // BOERSE_FRANKFURT_HPP_INCLUDED
