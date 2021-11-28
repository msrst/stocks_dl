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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <openssl/md5.h>
#include "boerse_frankfurt.hpp"
#include "utils/utils.hpp"
#include "stocks.hpp"
#include "curlwrapper.hpp"

std::string remove_multi_whitespaces(std::string str)
{
	std::string result = "";
	std::size_t pos = 0;
	while(true) {
		std::size_t new_pos_start = str.find(" ", pos);
		if(new_pos_start == std::string::npos) {
			result.append(str.substr(pos));
			return result;
		}

		std::size_t new_pos = new_pos_start;
		while((new_pos < str.size()) && (str[new_pos] == ' ')) {
			new_pos++;
		}
		result.append(str.substr(pos, new_pos_start - pos + 1));
		pos = new_pos;
	}
}

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

std::string my_md5(std::string str)
{
	unsigned char *result = MD5((const unsigned char*)str.c_str(), str.size(), NULL);
	return FormatToHex(std::basic_string<uint8_t>(result, 16), true, false);
}

std::map<std::string, std::string> CalcTokens(const std::string &url)
{
	const std::string api_salt = "w4icATTGtnjAQMbkL3kJwxLfEAKDa3VU";  // TODO check if this varies, else we have to get this from main js (tracing: { ... salt })

    using namespace boost::posix_time;
    ptime t = microsec_clock::universal_time();
    std::string client_date = to_iso_extended_string(t);
	const int comma_pos = sizeof("2021-11-06T19:47:39") - 1;
	client_date[comma_pos] = '.';
	client_date = client_date.substr(0, comma_pos + 4) + "Z";  // result should be 2021-11-06T19:47:39.398Z, for example

	std::string hash_input_string = client_date + url + api_salt;
	std::string client_trace_id = my_md5(hash_input_string);

	char buffer[sizeof("202111070726")];
	time_t t_local = time(NULL);
	std::strftime(buffer, sizeof(buffer), "%Y%m%d%H%M", localtime(&t_local));  // it is important to use localtime instead of UTC, so posix_time cannot be used
	std::string x_security = my_md5(buffer);

	std::map<std::string, std::string> headers;
	headers["Client-Date"] = client_date;
	headers["X-Client-TraceId"] = client_trace_id;
	headers["X-Security"] = x_security;  // it also works without x-security but it might be more safe to pass it as well
	return headers;
}

int price_downloader::Download(const std::string &isin, time_day date_min, time_day date_max, std::vector<daily_rate_nullable> &prices, std::string stock_exchange)
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

	//https://api.boerse-frankfurt.de/v1/data/price_history?limit=100000&offset=0&isin=DE0007236101&mic=XFRA&minDate=1970-03-26&maxDate=2020-03-26
	url = "https://api.boerse-frankfurt.de/v1/data/price_history?limit=100000&offset=0&isin=" +
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
	if(mcurl.GetSite(url, sitedata, false, CalcTokens(url))) {
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
	url = "https://api.boerse-frankfurt.de/v1/data/historical_key_data?isin=" + isin + "&limit=" + std::to_string(limit);
	STOCKS_DL_DEBUG(std::cout << url << std::endl);

	std::string api_data;
	if(mcurl.GetSite(url, api_data, false, CalcTokens(url))) {
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
	const std::string str_wkn_end = " | ISIN: " + isin + ")";
	size_t wkn_end = main_site_data.find(str_wkn_end);
	if(wkn_end == std::string::npos) {
		m_logger->LogError("Could not find WKN postfix \"" + str_wkn_end + "\".");
		return 4;
	}
	const std::string str_wkn_begin = "(WKN: ";
	size_t wkn_begin = main_site_data.rfind(str_wkn_begin, wkn_end);
	if(wkn_begin == std::string::npos) {
		m_logger->LogError("Could not find WKN prefix \"" + str_wkn_begin + "\"");
		return 5;
	}
	wkn_begin += str_wkn_begin.length();
	wkn = main_site_data.substr(wkn_begin, wkn_end - wkn_begin);
	STOCKS_DL_DEBUG(std::cout << "WKN: " << wkn << ", " << std::endl);
	if(wkn.length() > 10) {
		m_logger->LogError("WKN is " + Int_To_String(wkn.length()) + " characters long.");
		return 6;
	}
	wkn_end += str_wkn_end.length();

	const std::string str_kuerzel_begin = "| K\xc3\xbcrzel: ";
	size_t kuerzel_begin = main_site_data.find(str_kuerzel_begin);
	if(kuerzel_begin == std::string::npos) {
		m_logger->LogError(std::string("Could not find stock exchange symbol beginning \"") + str_kuerzel_begin + "\".");
		return 7;
	}
	kuerzel_begin += str_kuerzel_begin.size();
	const std::string str_kuerzel_end = "</span>";
	size_t kuerzel_end = main_site_data.find(str_kuerzel_end, kuerzel_begin);
	if(kuerzel_end == std::string::npos) {
		m_logger->LogError(std::string("Could not find stock exchange symbol ending \"") + str_kuerzel_end + "\".");
		return 7;
	}
	symbol_name = main_site_data.substr(kuerzel_begin, kuerzel_end - kuerzel_begin);
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

	//https://api.boerse-frankfurt.de/v1/data/dividend_information?isin=DE0007236101&limit=50
	url = "https://api.boerse-frankfurt.de/v1/data/dividend_information?isin=" +
			isin + "&limit=" + std::to_string(limit);
	STOCKS_DL_DEBUG(std::cout << "BF divdends " << url << std::endl);

	if(mcurl.GetSite(url, sitedata, false, CalcTokens(url))) {
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

int share_downloader::DownloadISINs(std::vector<isin_assignment> &isin_assignments, int max_page)
{
	for(int i_page = 0; i_page <= max_page; i_page++) {
		std::string url = "https://www.xetra.com/xetra-de/instrumente/aktien/liste-der-handelbaren-aktien/xetra/3002!search?sort=sTitle+asc&hitsPerPage=50&pageNum=" + std::to_string(i_page);
		std::string sitedata;
		if(mcurl.GetSite(url, sitedata)) {
			return 1;
		}
		std::cout << "setting file string " << sitedata.size() << std::endl;
		SetFileString("lala.html", sitedata);

		int count = 0;
		std::size_t current_pos = 0;
		while(true) {
			isin_assignment new_assignment;

			std::string href_str = "<h4><a href=\"/xetra-de/instrumente/aktien/liste-der-handelbaren-aktien/xetra/";
			std::size_t href_pos = sitedata.find(href_str, current_pos);
			if(href_pos == std::string::npos) {
				break;
			}

			std::string name_start_str = "\">";
			std::size_t name_start = sitedata.find(name_start_str, href_pos + href_str.size());
			if(name_start == std::string::npos) {
				m_logger->LogError("No name start string found.");
				return 3;
			}
			name_start += name_start_str.size();

			std::string name_end_str = "</a>";
			std::size_t name_end = sitedata.find(name_end_str, name_start);
			if(name_end == std::string::npos) {
				m_logger->LogError("No name end string found.");
				return 4;
			}
			new_assignment.name = sitedata.substr(name_start, name_end - name_start);
			if(new_assignment.name.size() > 200) {
				m_logger->LogWarning("name too long: " + new_assignment.name.substr(0, 200) + "...");
			}
			name_end;

			std::string isin_start_str = "<p>ISIN: ";
			std::size_t isin_start = sitedata.find(isin_start_str, name_end + name_end_str.size());
			if(isin_start == std::string::npos) {
				m_logger->LogError("No ISIN start string found.");
				return 5;
			}
			isin_start += isin_start_str.size();

			std::string isin_end_str = "</p><p>";
			std::size_t isin_end = sitedata.find(isin_end_str, isin_start);
			new_assignment.isin = sitedata.substr(isin_start, isin_end - isin_start);
			if(new_assignment.isin.size() > 30) {
				m_logger->LogWarning("ISIN too long: " + new_assignment.isin.substr(0, 30) + "...");
			}

			new_assignment.name = remove_multi_whitespaces(new_assignment.name);
			isin_assignments.push_back(new_assignment);
			current_pos = isin_end + isin_end_str.size();
			count++;
		}
		if(count != 50) {
			m_logger->LogWarning("Not the right number of stocks found: wanted 50, got " + std::to_string(count) + ".");
		}
	}
	return 0;
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
