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
#include <sstream>

#include "dividends.hpp"
#include "utils/utils.hpp"
#include "yahoodownloader.hpp"

namespace stocks_dl {

std::string ParseString_withuliterals(std::string str)
{
	
	std::string out;
	size_t begin = str.find("\\u00");
	while(begin != std::string::npos) {
		out += str.substr(0, begin);
		begin += 4;
		if(str.length() >= begin + 2) {
			out.append(1, Hex_To_uint8(str[begin], str[begin + 1]));
			str.erase(0, begin + 2);
			begin = str.find("\\u00");
		}
		else { // error
			std::cout << "Warning: \\u00xx: char number is to high." << std::endl;
			break;
		}
	}
	return out + str;
}

int yahoodownloader::Download(const stock_share &c_share, std::string ysymbol, time_day trading_day_min, time_day trading_day_max, std::vector<dividend> &dividends)
{
	if(!IsOk()) {
		return 1;
	}
	const std::string str_crumb_begin("\"CrumbStore\":{\"crumb\":\"");
	std::string sitedata;
	
	if(m_curlwrapper.ClearCookies()) {
		return 2;
	}
	if(m_curlwrapper.GetSite("https://de.finance.yahoo.com/quote/" + ysymbol
			+ "/history?period1=" + Int_To_String(trading_day_min.AsTimeT())
			+ "&period2=" + Int_To_String(trading_day_max.AsTimeT())
			+ "&interval=div%7Csplit&filter=div&frequency=1d", sitedata)) {
		return 3;
	}
	else {
		if(str_find_caseinsensitive(sitedata, c_share.name) == std::string::npos) {
			m_logger->LogError("The name of the share " + c_share.YSymbol() + " (calculated: " + ysymbol
					+ ") \"" + c_share.name + "\", id " + Int_To_String(c_share.id) + " was not found.");
			return 4;
		}
		size_t crumb_begin = sitedata.find(str_crumb_begin);
		if(crumb_begin == std::string::npos) {
			m_logger->LogError("Could not find the crumb begin " + str_crumb_begin + ".");
			return 5;
		}
		else {
			crumb_begin += str_crumb_begin.length();
			size_t crumb_end = sitedata.find('"', crumb_begin);
			if(crumb_end == std::string::npos) {
				m_logger->LogError("Could not find a Closing '\"' after the begin of the crumb.");
				return 6;
			}
			else {
				std::string crumb = ParseString_withuliterals(sitedata.substr(crumb_begin, crumb_end - crumb_begin));
				STOCKS_DL_DEBUG(std::cout << "Crumb = " << sitedata.substr(crumb_begin, crumb_end - crumb_begin) << " -> " << crumb << std::endl);
				std::string data;
				if(m_curlwrapper.GetSite("https://query1.finance.yahoo.com/v7/finance/download/" + ysymbol
						+ "?period1=" + Int_To_String(trading_day_min.AsTimeT())
						+ "&period2=" + Int_To_String(trading_day_max.AsTimeT())
						+ "&interval=1d&events=div&crumb=" + crumb, data)) {
					return 7;
				}
				else {
					std::stringstream sstream(data, std::ios_base::in);
					dividenden_csv c_dividend_csv;
					csv_reader::reader new_reader(',', &c_dividend_csv);
					int stat = new_reader.Parse(sstream);
					if(stat) {
						m_logger->LogError("CSV parser error: " + new_reader.GetError(stat));
						if(stat == csv_reader::reader::errors::CSVERR_FIRST_LINE_UNCOMPLETE) {
							m_logger->LogInformation("First line was: " + str_split(data, "\n")[0]); // str_split returns a vector with at minimum size 1
						}
					}
					else {
						dividends = c_dividend_csv.Get();
					}
					return 0;
				}
			}
		}
	}
}

} // namespace stocks_dl
