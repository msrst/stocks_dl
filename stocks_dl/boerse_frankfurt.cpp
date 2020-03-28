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

#include "boerse_frankfurt.hpp"
#include "utils/utils.hpp"
#include "stocks.hpp"
#include "curlwrapper.hpp"

namespace stocks_dl {
	
// Removes the rows of vec whose indexes are in row_indices.
// row_indices must be sorted so that the lowest index is the first.
// It is checked wether an index is given multiple.
template <class T>
void vector_remove_rows(std::vector<T> & vec, const std::vector<unsigned int> & row_indices)
{
	if(row_indices.empty() == false) {
		std::vector<unsigned int>::const_reverse_iterator itr_row = row_indices.rbegin();
		vec.erase(vec.begin() + (*itr_row));
		unsigned int last_invalid_row = *itr_row;
		for(; itr_row != row_indices.rend(); itr_row++) {
			if(*itr_row != last_invalid_row) {
				vec.erase(vec.begin() + (*itr_row));
				last_invalid_row = *itr_row;
			}
		}
	}
}

namespace boerse_frankfurt {

int price_downloader::Download(const std::string &isin, time_day date_min, time_day date_max, std::vector<daily_price> &prices, std::string stock_exchange)
{
	if(date_min.GetYear() < 1970) {
		m_logger->LogWarning("Please not that boerse frankfurt does return only prices from 2000 and above if you specify a year < 1970.");
	}

	//if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, slist_reset)) { // Deletes the three http header options used by former calls of this function
	if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, NULL)) { // Deletes the three http header options used by former calls of this function
		m_logger->LogError("Price Downloader: Could not set curl option httpheader to remove header options.");
		return 2;
	}
	
	std::string sitedata, url;

	//https://api.boerse-frankfurt.de/data/price_history?limit=100000&offset=0&isin=DE0007236101&mic=XFRA&minDate=1970-03-26&maxDate=2020-03-26
	url = "https://api.boerse-frankfurt.de/data/price_history?limit=100000&offset=0&isin=" + 
			isin + "&mic=" + stock_exchange + "&minDate=" + date_min.AsString() + 
			"&maxDate=" + date_max.AsString();
	STOCKS_DL_DEBUG(std::cout << url << std::endl);

	/*if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_COOKIEFILE, "")) { // Enable cookies
		std::cout << "Error 2." << std::endl;
	}
	else if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_COOKIEJAR, "cookies.txt")) {
		std::cout << "Error 3." << std::endl;
	}
	else {*/
	if(mcurl.GetSite(url, sitedata)) {
		return 3;
	}
	SetFileString("out.json", sitedata);

	if(m_bf_converter->PricesFromJson(sitedata, prices) != 0) {
		if(sitedata.size() < 1000) {
			// sometimes the json gives you a hint what was wrong, e. g. when specifying an invalid isin
			m_logger->LogError("JSON string was: " + sitedata);
		}
		return 4;
	}
	else {
		return 0;
	}
}

int share_downloader::Download(long int share_id, const std::string &isin, std::vector<balance_sheet> &balance_sheets, std::string &wkn, std::string &symbol_name, int limit, std::string stock_exchange)
{
	std::string url;
	url = "https://api.boerse-frankfurt.de/data/historical_key_data?isin=" + isin + "&limit=" + std::to_string(limit);
	STOCKS_DL_DEBUG(std::cout << url << std::endl);
	
	std::string api_data;
	if(mcurl.GetSite(url, api_data)) {
		return 1;
	}

	std::string bfname;
	if(!m_bf_converter->GetBFName_ViaSearch(isin, mcurl, bfname)) {
		return 2;
	}

	std::string main_site_data;
	// https://www.boerse-frankfurt.de/aktie/siemens-ag
	std::string main_url = "https://www.boerse-frankfurt.de/aktie/" + bfname;
	if(mcurl.GetSite(main_url, main_site_data)) {
		return 3;
	}
	const std::string str_wkn_beginn = "ISIN: " + isin + " | WKN: ", str_wkn_end = " | K\xc3\xbcrzel: ";
	size_t wkn_beginn = main_site_data.find(str_wkn_beginn);
	if(wkn_beginn == std::string::npos) {
		m_logger->LogError("Could not find WKN prefix \"" + str_wkn_beginn + "\".");
		return 4;
	}
	wkn_beginn += str_wkn_beginn.length();
	size_t wkn_end = main_site_data.find(str_wkn_end, wkn_beginn);
	if(wkn_end == std::string::npos) {
		m_logger->LogError("Could not find WKN postfix \"" + str_wkn_end + "\"");
		return 5;
	}
	wkn = main_site_data.substr(wkn_beginn, wkn_end - wkn_beginn);
	STOCKS_DL_DEBUG(std::cout << "WKN: " << wkn << ", " << std::endl);
	if(wkn.length() > 10) {
		m_logger->LogError("WKN ist " + Int_To_String(wkn.length()) + " Zeichen lang.");
		return 6;
	}
	wkn_end += str_wkn_end.length();
	size_t kuerzel_end = main_site_data.find(" | Typ:", wkn_end);
	if(kuerzel_end == std::string::npos) {
		m_logger->LogError(std::string("Could not find stock exchange symbol ending \"") + " | Typ:" + std::string("\"."));
		return 7;
	}
	symbol_name = main_site_data.substr(wkn_end, kuerzel_end - wkn_end);
	STOCKS_DL_DEBUG(std::cout << "Symbol-Name: " << symbol_name << std::endl);
	if(symbol_name.length() > 6) {
		m_logger->LogError("The length of the stock exchange symbol is " + Int_To_String(symbol_name.length()) + ".");
		return 8;
	}
	
	if(m_bf_converter->BalanceSheetFromJson(share_id, api_data, balance_sheets)) {
		if(api_data.length() < 1000) {
			m_logger->LogError("JSON string was: " + api_data);
		}
		return 9;
	}
	else {
		return 0;
	}
}
int share_downloader::DownloadDividends(const std::string &isin, std::vector<dividend> &dividends, int limit, std::string stock_exchange)
{
	//if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, slist_reset)) { // Deletes the three http header options used by former calls of this function
	if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, NULL)) { // Deletes the three http header options used by former calls of this function
		m_logger->LogError("Price Downloader: Could not set curl option httpheader to remove header options.");
		return 2;
	}
	
	std::string sitedata, url;

	//https://api.boerse-frankfurt.de/data/dividend_information?isin=DE0007236101&limit=50
	url = "https://api.boerse-frankfurt.de/data/dividend_information?isin=" + 
			isin + "&limit=" + std::to_string(limit);
	STOCKS_DL_DEBUG(std::cout << "BF divdends " << url << std::endl);

	if(mcurl.GetSite(url, sitedata)) {
		return 3;
	}
	SetFileString("out.json", sitedata);

	if(m_bf_converter->DividendsFromJson(isin, sitedata, dividends) != 0) {
		if(sitedata.size() < 1000) {
			// sometimes the json gives you a hint what was wrong, e. g. when specifying an invalid isin
			m_logger->LogError("JSON string was: " + sitedata);
		}
		return 4;
	}
	else {
		return 0;
	}
}

} // namespace boerse_frankfurt

bool FindAJAXString(const std::string &sitedata, const std::string &str_begin, std::string &ajax_str, logger_base_ptr c_logger)
{
	size_t begin = sitedata.find(str_begin);
	if(begin == std::string::npos) {
		c_logger->LogError("Could not find " + str_begin + ".");
		return false;
	}
	else {
		begin += str_begin.length();
		size_t end = sitedata.find('"', begin);
		if(end == std::string::npos) {
			c_logger->LogError("Could not find a closing '\"' after " + str_begin + ".");
			return false;
		}
		else {
			ajax_str = sitedata.substr(begin, end - begin);
			return true;
		}
	}
}

} // namespace stocks_dl
