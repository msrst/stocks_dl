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
#include "utils/calculator.hpp"

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

void balance_sheet::Print()
{
	std::cout << year << ", currency: " << currency << std::endl;
	for(unsigned int i1 = 0; i1 < BALANCE_SHEET_FIELD_COUNT; i1++) {
		if(data[i1]) {
			std::cout << field_names[i1]
				<< std::string(field_names_max_len + 1 - field_names[i1].length(), ' ') << data[i1] << std::endl;
		}
	}
}

bf_converter::bf_converter(logger_base_ptr c_logger):
	currency_balance_sheet_field(		 "Bilanzierungsw\xc3\xa4hrung")
{
	m_logger = c_logger;
	
	// In the geany editor, user Strg-Shift-Alt to select a text in block mode
	balance_sheet_fields.emplace_back("Umsatzerlöse in Mio.");
	balance_sheet_fields.emplace_back("Dividende");
	balance_sheet_fields.emplace_back("Dividendenrendite (in %)", true);
	balance_sheet_fields.emplace_back("Ergebnis/Aktie");
	balance_sheet_fields.emplace_back("KGV");
	balance_sheet_fields.emplace_back("EBIT in Mio.");
	balance_sheet_fields.emplace_back("EBITDA in Mio.");
	balance_sheet_fields.emplace_back("Gewinn in Mio.");
	balance_sheet_fields.emplace_back("Gewinn (bereinigt) in Mio.");
	balance_sheet_fields.emplace_back("Gewinn (vor Steuern) in Mio.");
	balance_sheet_fields.emplace_back("Gewinn (vor Steuern, reported) in Mio.");
	balance_sheet_fields.emplace_back("Ergebnis/Aktie (bereinigt)");
	balance_sheet_fields.emplace_back("Ergebnis/Aktie (reported)");
	balance_sheet_fields.emplace_back("Bruttoergebnis vom Umsatz in Mio.");
	balance_sheet_fields.emplace_back("Cashflow (Investing) in Mio.");
	balance_sheet_fields.emplace_back("Cashflow (Operations) in Mio.");
	balance_sheet_fields.emplace_back("Cashflow (Financing) in Mio.");
	balance_sheet_fields.emplace_back("Cashflow/Aktie");
	balance_sheet_fields.emplace_back("Free Cashflow in Mio.");
	balance_sheet_fields.emplace_back("Free Cashflow/Aktie");
	balance_sheet_fields.emplace_back("Buchwert/Aktie"); 					// Eigenkapital pro Aktie
	balance_sheet_fields.emplace_back("Gesamtverbindlichkeiten in Mio.");
	balance_sheet_fields.emplace_back("Ausgaben (Research & Development) in Mio.");
	balance_sheet_fields.emplace_back("Kapitalausgaben in Mio.");
	balance_sheet_fields.emplace_back("Ausgaben (Selling, General & Administration) in Mio.");
	balance_sheet_fields.emplace_back("Eigenkapital in Mio.");
	balance_sheet_fields.emplace_back("Bilanzsumme in Mio.");
	if(balance_sheet_fields.size() != BALANCE_SHEET_FIELD_COUNT) {
		m_logger->LogError("Zuordnungen.size() = " + Int_To_String(balance_sheet_fields.size()) + ".");
	}
	
	for(unsigned int i1 = 0; i1 < balance_sheet_fields.size(); i1++) {
		bf_to_num[balance_sheet_fields[i1].bf_name] = i1;
	}
	
	bf_kurs_heading_heading = "Datum";
	bf_price_headings.push_back("Er\xc3\xb6""ffnung");
	bf_price_headings.push_back("Schluss");
	bf_price_headings.push_back("Tageshoch");
	bf_price_headings.push_back("Tagestief");
	bf_price_headings.push_back("Umsatz in \xe2\x82\xac"); // Euro
	bf_price_headings.push_back("Umsatz in St\xc3\xbc""ck");
	
	bf_dividend_heading_heading = "Letzte Auszahlung";
	bf_dividend_headings.push_back("Dividendenzyklus");
	bf_dividend_headings.push_back("Betrag");
	bf_dividend_headings.push_back("ISIN");
}

int bf_converter::BalanceSheetFromTable(long int stock_id, const html_table &table, std::vector<balance_sheet> &balance_sheets)
{
	string_to_int_limited_converter convertobj;
	std::vector<balance_sheet> res(table.heading_row.size());
	for(unsigned int i1 = 0; i1 < res.size(); i1++) {
		res[i1].stock_id = stock_id;
		res[i1].year = convertobj.ConvertToLLI_Limited(table.heading_row[i1], 1900, 3000);
	}
	if(convertobj.MistakeHappened() || convertobj.LimitMistakeHappened()) {
		m_logger->LogError("Balance sheet reader: Could not convert the year date.");
		return 1;
	}
	for(unsigned int i1 = 0; i1 < table.zeilen.size(); i1++) {
		std::map<std::string, unsigned int>::iterator it_num = bf_to_num.find(table.heading_column[i1]);
		if(it_num == bf_to_num.end()) {
			if(table.heading_column[i1] == currency_balance_sheet_field.bf_name) {
				for(unsigned int i2 = 0; i2 < table.zeilen[i1].size(); i2++) {
					res[i2].currency = table.zeilen[i1][i2];
				}
			}
			else {
				m_logger->LogWarning("Balance sheet reader: Could not use row \"" + table.heading_column[i1] + "\".");
			}
		}
		else {
			for(unsigned int i2 = 0; i2 < table.zeilen[i1].size(); i2++) {
				if(!(table.zeilen[i1][i2].empty() || (balance_sheet_fields[it_num->second].is_percentage && (table.zeilen[i1][i2] == "%")))) {
					int fehler = convertobj.GetError();
					if(balance_sheet_fields[it_num->second].is_percentage && (str_ends_with(table.zeilen[i1][i2], " %"))) {
						res[i2].data[it_num->second].FromString(convertobj, table.zeilen[i1][i2].substr(0, table.zeilen[i1][i2].length() - 2), ',', '.');
					}
					else {
						res[i2].data[it_num->second].FromString(convertobj, table.zeilen[i1][i2], ',', '.');
					}
					if(convertobj.GetError() > fehler) {
						m_logger->LogWarning("Balance sheet reader: Could not convert the number \"" + table.zeilen[i1][i2] + "\" in row " + Int_To_String(i1)
								+ " (" + table.heading_column[i1] + "), column " + Int_To_String(i2) + ".");
					}
				}
			}
		}
	}
	balance_sheets.insert(balance_sheets.end(), res.begin(), res.end());
	return 0;
}
int bf_converter::PricesFromTable(const html_table &table, std::vector<daily_price> &prices)
{
	if(table.heading_heading != bf_kurs_heading_heading) {
		m_logger->LogError("Price reader: The heading of the heading row is invalid:");
		m_logger->LogError("it is: " + table.heading_heading + ", but it should be: " + bf_kurs_heading_heading);
		return 1;
	}
	if(table.heading_row != bf_price_headings) {
		m_logger->LogError("Price reader: The heading row is incorrect.");
		m_logger->LogError("it is:  " + vectorstring_unsplit(table.heading_row, '|'));
		m_logger->LogError("it should be:  " + vectorstring_unsplit(bf_price_headings, '|'));
		return 1;
	}
	prices.clear();
	prices.resize(table.zeilen.size());
	std::vector<unsigned int> invalid_rows;
	for(unsigned int irow = 0; irow < table.zeilen.size(); irow++) {
		bool b_ok = false;
		prices[irow].handelstag = TimeDayFromDEString(table.heading_column[irow], &b_ok);
		if(!b_ok) {
			m_logger->LogWarning("Pricer reader: Could not convert the date \"" + table.heading_column[irow] + "\" in row " + Int_To_String(irow)
					+ ".");
			invalid_rows.push_back(irow);
		}
		for(unsigned int icolumn = 0; icolumn < table.heading_row.size(); icolumn++) {
			string_to_int_converter convertobj;
			switch(icolumn) {
			 case 0: // Eröffnung
				prices[irow].price_open = convertobj.ConvertToDouble(table.zeilen[irow][icolumn], ',', '.');
				break;
			 case 1: // Schluss
				prices[irow].price_closing = convertobj.ConvertToDouble(table.zeilen[irow][icolumn], ',', '.');
				break;
			 case 2: // Tageshoch
				prices[irow].price_high = convertobj.ConvertToDouble(table.zeilen[irow][icolumn], ',', '.');
				break;
			 case 3: // Tagestief
				prices[irow].price_low = convertobj.ConvertToDouble(table.zeilen[irow][icolumn], ',', '.');
				break;
			 // case 4: Umsatz in €
			 case 5: // Umsatz in Stück
				prices[irow].volume.FromString(convertobj, table.zeilen[irow][icolumn], ',', '.');
				break;
			}
			if(convertobj.MistakeHappened() && (icolumn != 5)) {
				m_logger->LogWarning("Price reader: Could not convert the number \"" + table.zeilen[irow][icolumn] + "\" in row " + Int_To_String(irow)
						+ " (" + table.heading_column[irow] + "), column " + Int_To_String(icolumn) + ".");
				invalid_rows.push_back(irow);
			}
		}
	}
	vector_remove_rows(prices, invalid_rows);
	return 0;
}
int bf_converter::DividendsFromTable(std::string isin, const html_table &table, std::vector<dividend> &dividends)
{
	if(table.heading_heading != bf_dividend_heading_heading) {
		m_logger->LogError("Dividend reader: The heading of the heading row is invalid: ");
		m_logger->LogError("it is: " + table.heading_heading + ", but it should be: " + bf_dividend_heading_heading);
		return 1;
	}
	if(table.heading_row != bf_dividend_headings) {
		m_logger->LogError("Dividend reader: The heading row is incorrect.");
		m_logger->LogError("it is:  " + vectorstring_unsplit(table.heading_row, '|'));
		m_logger->LogError("it should be:  " + vectorstring_unsplit(bf_dividend_headings, '|'));
		return 2;
	}
	dividends.clear();
	dividends.resize(table.zeilen.size());
	std::vector<unsigned int> invalid_rows;
	for(unsigned int irow = 0; irow < table.zeilen.size(); irow++) {
		bool b_ok = false;
		dividends[irow].date = TimeDayFromDEString(table.heading_column[irow], &b_ok);
		if(!b_ok) {
			m_logger->LogWarning("Pricer reader: Could not convert the date \"" + table.heading_column[irow] + "\" in row " + Int_To_String(irow)
					+ ".");
			invalid_rows.push_back(irow);
		}
		for(unsigned int icolumn = 0; icolumn < table.heading_row.size(); icolumn++) {
			string_to_int_converter convertobj;
			switch(icolumn) {
			 // case 0: // dividend cycle (yearly / half-year...) (not needed)
			 case 1: // amount, format: "€ 25.3"
				if(str_begins_with(table.zeilen[irow][icolumn], "\xe2\x82\xac ")) { // "€ " in UTF-8
					dividends[irow].dividend = convertobj.ConvertToDouble(table.zeilen[irow][icolumn].substr(4, 
							table.zeilen[irow][icolumn].length() - 4), ',', '.');
				}
				else {
					m_logger->LogWarning("Dividend reader: Could not read the dividend \"" + table.zeilen[irow][icolumn]
							+ "\" in row " + Int_To_String(irow) + " (" + table.heading_column[irow] + ").");
					invalid_rows.push_back(irow);
				}
				break;
			 case 2: // ISIN
				if(table.zeilen[irow][icolumn] != isin) {
					// Sometimes, dividends other share variants (like preference shares) are also listed by boerse_frankfurt.de. Ignore them.
					m_logger->LogInformation("Ignored dividend row with isin " + table.zeilen[irow][icolumn] 
							+ " of the share with isin \"" + isin + "\".");
					invalid_rows.push_back(irow);
				}
			}
			if(convertobj.MistakeHappened()) {
				m_logger->LogWarning("Dividend reader: Could not convert the number \"" + table.zeilen[irow][icolumn] + "\" in row " + Int_To_String(irow)
						+ " (" + table.heading_column[irow] + "), column " + Int_To_String(icolumn) + ".");
				invalid_rows.push_back(irow);
			}
		}
	}
	vector_remove_rows(dividends, invalid_rows);
	// Sometimes, boerse_frankfurt separates yearly dividends and one-time dividends. Put them together.
	for(unsigned int i1 = 1; i1 < dividends.size();) {
		if(dividends[i1].date == dividends[i1 - 1].date) {
			dividends[i1 - 1].dividend += dividends[i1].dividend;
			dividends.erase(dividends.begin() + i1);
			m_logger->LogInformation("Summed up two dividends of " + dividends[i1 - 1].date.AsString() + ".");
		}
		else {
			i1++;
		}
	}
	return 0;
}

bool bf_converter::GetBFName_ViaSearch(const std::string &isin, mcurlwrapper &mcurl, std::string &bfname)
{
	if(!mcurl.IsOk()) {
		m_logger->LogError("Curl not initialised!");
		return false;
	}
	const std::string aktienstart_begin = "http://www.boerse-frankfurt.de/aktie/";
	std::string sitedata, redirect_url;
	STOCKS_DL_DEBUG(std::cout << std::string("http://www.boerse-frankfurt.de/suchergebnisse?_search=") + isin << std::endl);
	if(mcurl.GetRedirect(std::string("http://www.boerse-frankfurt.de/suchergebnisse?_search=") + isin, redirect_url)) {
		return false;
	}
	STOCKS_DL_DEBUG(std::cout << "redirect: " << redirect_url << std::endl);
	if(!str_begins_with(redirect_url, aktienstart_begin)) {
		m_logger->LogError("Redirect does not begin with \"" + aktienstart_begin + "\".");
		return false;
	}
	size_t bfname_end = redirect_url.find("-Aktie", aktienstart_begin.length());
	if(bfname_end == std::string::npos) {
		m_logger->LogError("Could not find \"-Aktie\".");
		return false;
	}
	bfname = redirect_url.substr(aktienstart_begin.length(), bfname_end - aktienstart_begin.length());
	return true;
	
	/*
	const std::string str_bfname_begin("\", \"");
	std::string getsite_url = "http://www.boerse-frankfurt.de/ajax/SearchController_Suggest/?max_results=25&Keywords_mode=APPROX&Keywords="
			+ isin + "&query=" + isin + "&bias=100&target_id=0";
	STOCKS_DL_DEBUG(std::cout << getsite_url << std::endl);
	if(mcurl.GetSite(getsite_url, sitedata, true)) {
		return false;
	}
	// Bsp.:
	// mmSuggestDeliver(0, new Array("Name", "Category", "Keywords", "Bias", "Extension", "IDs"), new Array(new Array("BASF", "Aktien", "BASF11|DE000BASF111|||BASFn", "95", "", "BASF|BASF11|1|2075775")), 1, 0);
	STOCKS_DL_DEBUG(std::cout << sitedata.length() << std::endl);
	if(sitedata.length() > 10000) {
		m_logger->LogError("Sitedata too long.");
		return false;
	}
	SetFileString("out_ajax.html", sitedata);
	size_t bfname_begin = sitedata.rfind(str_bfname_begin);
	if(bfname_begin == std::string::npos) {
		m_logger->LogError("Could not find name beginning \"" + str_bfname_begin + "\".");
		return false;
	}
	bfname_begin += str_bfname_begin.size();
	size_t bfname_end = sitedata.find('|', bfname_begin);
	if(bfname_end == std::string::npos) {
		m_logger->LogError("Could not find a closing '|'.");
		return false;
	}
	bfname = sitedata.substr(bfname_begin, bfname_end - bfname_begin);
	STOCKS_DL_DEBUG(std::cout << "BFName: " << bfname << std::endl);
	return true;*/
}
bool bf_converter::GetBFName_ViaStandard(const std::string &simple_name, mcurlwrapper &mcurl, std::string &bfname)
{
	const std::string str_bflink_begin = "<a href=\"/aktie";
	std::string sitedata;
	std::string beginn_buchstabe;
	if((simple_name[0] >= '0') && (simple_name[0] <= '9')) {
		beginn_buchstabe = "0-9";
	}
	else if((simple_name[0] >= 'A') && (simple_name[0] <= 'Z')) {
		beginn_buchstabe = simple_name[0];
	}
	else if((simple_name[0] >= 'a') && (simple_name[0] <= 'z')) {
		beginn_buchstabe = simple_name[0] - 'a' + 'A';
	}
	else {
		m_logger->LogError("The beginning letter of the share \"" + simple_name + "\" could not be determined.");
		return false;
	}
	std::string url_primelinks = "http://www.boerse-frankfurt.de/aktien/primestandard/" + beginn_buchstabe;
	STOCKS_DL_DEBUG(std::cout << url_primelinks << std::endl);
	if(mcurl.GetSite(url_primelinks, sitedata)) {
		return false;
	}
	STOCKS_DL_DEBUG(std::cout << sitedata.length() << std::endl);
	std::string search = "\" title=\"" + simple_name;
	std::string search2 = "\" title=\"" + str_to_upper(simple_name);
	size_t bfname_end = sitedata.find(search);
	if(bfname_end == std::string::npos) {
		bfname_end = sitedata.find(search2);
	}
	if(bfname_end == std::string::npos) {
		m_logger->LogError("Neither \"" + search + "\" nor \"" + search2 + "\" could be found.");
		return false;
	}
	else if(bfname_end == 0) {
		m_logger->LogError("Name_end = 0");
		return false;
	}
	size_t bfname_begin = sitedata.rfind(str_bflink_begin, bfname_end - 1);
	if(bfname_begin == std::string::npos) {
		m_logger->LogError("Could not find \"" + str_bflink_begin + "\".");
		return false;
	}
	bfname_begin += str_bflink_begin.length();
	bfname = sitedata.substr(bfname_begin, bfname_end - bfname_begin);
	if((!str_ends_with(bfname, "-Aktie")) || (!str_begins_with(bfname, "/"))) {
		m_logger->LogError(bfname + " does not end with -Aktie or does not begin with /.");
		return false;
	}
	bfname.erase(bfname.length() - 6, 6);
	bfname.erase(0, 1);
	return true;
}

int price_downloader::Download(const std::string &isin, std::string date_span, std::vector<daily_price> &prices, std::string stock_exchange)
{
	//if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, slist_reset)) { // Deletes the three http header options used by former calls of this function
	if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, NULL)) { // Deletes the three http header options used by former calls of this function
		m_logger->LogError("Price Downloader: Could not set curl option httpheader to remove header options.");
		return 2;
	}
	
	std::string sitedata;
	std::string redirect_url, bfname;
	if(!m_bf_converter->GetBFName_ViaSearch(isin, mcurl, bfname)) {
		return 1;
	}
	redirect_url = "http://www.boerse-frankfurt.de/aktie/kurshistorie/" + bfname + "-Aktie/" + stock_exchange + "/" + date_span + "#Kurshistorie";
	STOCKS_DL_DEBUG(std::cout << redirect_url << std::endl);

	/*if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_COOKIEFILE, "")) { // Enable cookies
		std::cout << "Error 2." << std::endl;
	}
	else if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_COOKIEJAR, "cookies.txt")) {
		std::cout << "Error 3." << std::endl;
	}
	else {*/
	if(mcurl.GetSite(redirect_url, sitedata)) {
		return 3;
	}
	STOCKS_DL_DEBUG(std::cout << sitedata.length() << std::endl);
	SetFileString("out.html", sitedata);
	/*
	curl_slist *cookies;
	curl_easy_getinfo(mcurl.easy_handle, CURLINFO_COOKIELIST, &cookies);
	if(cookies) {
		mcurl_print_slist(cookies);
		curl_slist_free_all(cookies);
	}
	else {
		std::cout << "no cookies." << std::endl;
	}*/
	
	// also possible with curl 'http://www.boerse-frankfurt.de/Ajax/SharesCler_HistoricPriceList/BASF-Aktie/FSE/14.12.2018_29.12.2018' -H 'User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:64.0) Gecko/20100101 Firefox/64.0' -H '__atts: 2018-12-29-20-17-47' -H '__ath: +EOvAZtfvS25OFJAivPta5S+mnCaAHjiU+wZRaMiBEE=' -H '__atcrv: 53731178' --data ''
	std::string ajax__atts, ajax__ath, ajax__atcrv;
	if((!FindAJAXString(sitedata, "<input class=\"ajax-token\" name=\"__atts\" type=\"hidden\" value=\"", ajax__atts, m_logger))
			|| (!FindAJAXString(sitedata, "<input class=\"ajax-token\" name=\"__ath\" type=\"hidden\" value=\"", ajax__ath, m_logger))
			|| (!FindAJAXString(sitedata, "<input class=\"ajax-token\" name=\"__atcrv\" type=\"hidden\" value=\"", ajax__atcrv, m_logger))) {
		return 4;
	}
	
	std::string requestedStockMarket = stock_exchange;
	std::vector<std::string> decodedUri = str_split(redirect_url, "/");
	std::string instrumentUrl = decodedUri[decodedUri.size() - 3];
	std::string url = "http://www.boerse-frankfurt.de/Ajax/SharesController_HistoricPriceList/" + instrumentUrl + '/' + requestedStockMarket + '/' + date_span;
	double result;
	if(!fn_calculate(ajax__atcrv, m_logger, result)) {
		return 5;
	}
	curl_slist* slist = NULL;
	std::string ajax_eval_atcrv = Int_To_String(result);
	STOCKS_DL_DEBUG(std::cout << ajax__atcrv << " = " << ajax_eval_atcrv << std::endl);
	slist = curl_slist_append(NULL, ("__atcrv: " + ajax_eval_atcrv).c_str());
	curl_slist_append(slist, ("__ath: " + ajax__ath).c_str());
	curl_slist_append(slist, ("__atts: " + ajax__atts).c_str());
	STOCKS_DL_DEBUG(mcurl_print_slist(slist));
	if(curl_easy_setopt(mcurl.easy_handle, CURLOPT_HTTPHEADER, slist)) { // Curl does not copy the slist. Therefore 
										// it must be hold until the last curl call
		m_logger->LogError("Could not set the curl option httpheader.");
		curl_slist_free_all(slist);
		return 6;
	}
	STOCKS_DL_DEBUG(std::cout << url << std::endl);
	
	if(mcurl.GetSitePost(url, "", sitedata)) {
		curl_slist_free_all(slist);
		return 7;
	}
	curl_slist_free_all(slist);
	STOCKS_DL_DEBUG(std::cout << sitedata.length() << std::endl);
	//std::cout << sitedata << std::endl);
	STOCKS_DL_DEBUG(SetFileString("out2.html", sitedata));
	long int response_code;
	curl_easy_getinfo(mcurl.easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
	STOCKS_DL_DEBUG(std::cout << "Response Code " << response_code << std::endl);
	size_t table_begin = sitedata.find("<table class=\"table\">");
	if(table_begin == std::string::npos) {
		m_logger->LogError("Price Downloader: Could not find the beginning " "<table class=\"table\">" " of the table.");
		return 8;
	}
	const std::string str_table_end("</table>");
	size_t table_end = sitedata.find(str_table_end, table_begin);
	if(table_end == std::string::npos) {
		m_logger->LogError("Price Downloader: Could not find the ending " + str_table_end + " of the table.");
		return 9;
	}
	table_end += str_table_end.length();
	//std::cout << std::string(sitedata.begin() + table_begin, sitedata.begin() + table_end) << std::endl);
	
	html_table c_table;
	if(html_table_parser::Parse(sitedata.begin() + table_begin, sitedata.begin() + table_end, m_logger, c_table, true)) {
		return 10;
	}
	if(!c_table.Check(m_logger, true)) {
		return 11;
	}
	c_table.RemoveSidewhites();
	//c_table.Print();
	m_bf_converter->PricesFromTable(c_table, prices);
	return 0;
}

int share_downloader::Download(long int share_id, const std::string &isin, std::vector<balance_sheet> &balance_sheets, std::string &wkn, std::string &symbol_name, std::string stock_exchange)
{
	/*
	std::string redirect_url;
	if(mcurl.GetRedirect(std::string("http://www.boerse-frankfurt.de/suchergebnisse?_search=") + isin, redirect_url)) {
		return 1;
	}
	STOCKS_DL_DEBUG(std::cout << "redirect: " << redirect_url << std::endl);
	const std::string aktienstart_begin = "http://www.boerse-frankfurt.de/aktie/";
	if(!str_begins_with(redirect_url, aktienstart_begin)) {
		m_logger->LogError("Redirect does not begin with \"" + aktienstart_begin + "\".");
		return 2;
	}
	redirect_url.insert(aktienstart_begin.length(), "kennzahlen/");
	std::string sitedata;
	STOCKS_DL_DEBUG(std::cout << redirect_url + "/FSE#Kennzahlen" << std::endl);
	if(mcurl.GetSite(redirect_url + "/FSE#Kennzahlen", sitedata)) {
		return 3;
	}*/
	
	std::string redirect_url, bfname;
	if(!m_bf_converter->GetBFName_ViaSearch(isin, mcurl, bfname)) {
		return 1;
	}
	redirect_url = "http://www.boerse-frankfurt.de/aktie/kennzahlen/" + bfname + "-Aktie/" + stock_exchange + "#Kennzahlen";
	STOCKS_DL_DEBUG(std::cout << redirect_url << std::endl);
	
	std::string sitedata;
	if(mcurl.GetSite(redirect_url, sitedata)) {
		return 3;
	}
	STOCKS_DL_DEBUG(std::cout << sitedata.length() << std::endl);
	std::string str_wkn_beginn = "ISIN " + isin + " | WKN ", str_wkn_end = " | K\xc3\xbcrzel ";
	size_t wkn_beginn = sitedata.find(str_wkn_beginn);
	if(wkn_beginn == std::string::npos) {
		m_logger->LogError("Could not find WKN prefix \"" + str_wkn_beginn + "\".");
		return 4;
	}
	wkn_beginn += str_wkn_beginn.length();
	size_t wkn_end = sitedata.find(str_wkn_end, wkn_beginn);
	if(wkn_end == std::string::npos) {
		m_logger->LogError("Could not find WKN postfix \"" + str_wkn_end + "\"");
		return 5;
	}
	wkn = sitedata.substr(wkn_beginn, wkn_end - wkn_beginn);
	STOCKS_DL_DEBUG(std::cout << "WKN: " << wkn << ", " << std::endl);
	if(wkn.length() > 10) {
		m_logger->LogError("WKN ist " + Int_To_String(wkn.length()) + " Zeichen lang.");
		return 6;
	}
	wkn_end += str_wkn_end.length();
	size_t kuerzel_end = sitedata.find(" | Typ Aktie", wkn_end);
	if(kuerzel_end == std::string::npos) {
		m_logger->LogError(std::string("Could not find stock exchange symbol ending \"") + " | Typ Aktie" + std::string("\"."));
		return 7;
	}
	symbol_name = sitedata.substr(wkn_end, kuerzel_end - wkn_end);
	STOCKS_DL_DEBUG(std::cout << "Symbol-Name: " << symbol_name << std::endl);
	if(symbol_name.length() > 6) {
		m_logger->LogError("The length of the stock exchange symbol is " + Int_To_String(symbol_name.length()) + ".");
		return 8;
	}
	
	//SetFileString("out.html", sitedata);
	size_t table_begin = sitedata.find("<table class=\"table balance-sheet\">");
	if(table_begin == std::string::npos) {
		if(sitedata.find("Zu diesem Datensatz liegen uns leider keine Daten vor.") != std::string::npos) { // They simply have no balance sheet data for this stock
			m_logger->LogInformation("Boerse-frankfurt.de has no data for the share with isin " + isin + ".");
			return 0;
		}
		else {
			m_logger->LogError("Could not find table beginning \"" "<table class=\"table balance-sheet\">" "\".");
			return 9;
		}
	}
	else {
		const std::string str_table_end("</table>");
		size_t table_end = sitedata.find(str_table_end, table_begin);
		if(table_end == std::string::npos) {
			m_logger->LogError("Could not find table ending \"" + str_table_end + "\".");
			return 10;
		}
		else {
			table_end += str_table_end.length();
			html_table c_table;
			if(html_table_parser::Parse(sitedata.begin() + table_begin, sitedata.begin() + table_end, m_logger, c_table)) {
				return 11;
			}
			if(!c_table.Check(m_logger)) {
				return 12;
			}
			STOCKS_DL_DEBUG(c_table.Print());
			m_bf_converter->BalanceSheetFromTable(share_id, c_table, balance_sheets);
			return 0;
		}
	}
}
int share_downloader::DownloadDividends(const std::string &isin, std::vector<dividend> &dividends, std::string stock_exchange)
{
	std::string redirect_url, bfname;
	if(!m_bf_converter->GetBFName_ViaSearch(isin, mcurl, bfname)) {
		return 1;
	}
	// Example URL: http://www.boerse-frankfurt.de/aktie/unternehmensangaben/Siemens-Aktie/FSE#Unternehmensangaben
	redirect_url = "http://www.boerse-frankfurt.de/aktie/unternehmensangaben/" + bfname + "-Aktie/" + stock_exchange + "#Unternehmensangaben";
	STOCKS_DL_DEBUG(std::cout << redirect_url << std::endl);
	
	std::string sitedata;
	if(mcurl.GetSite(redirect_url, sitedata)) {
		return 2;
	}
	STOCKS_DL_DEBUG(std::cout << sitedata.length() << std::endl);
	//SetFileString("out.html", sitedata);
	
	size_t table_begin = sitedata.find("<h2>Dividende</h2>");
	if(table_begin == std::string::npos) {
		m_logger->LogWarning("Could not find the dividends table. Maybe boerse-frankfurt.de has no data for " + bfname + " shares.");
		return 3;
	}
	table_begin = sitedata.find("<table class=\"table\">", table_begin);
	if(table_begin == std::string::npos) {
		// When there is a dividend heading, there must be normally a table begin.
		m_logger->LogError("Could not find the table begin \"" + std::string("<table class=\"table-responsive\">") + "\".");
		return 4;
	}
	const std::string str_table_end("</table>");
	size_t table_end = sitedata.find(str_table_end, table_begin);
	if(table_end == std::string::npos) {
		m_logger->LogError("Could not find the table ending \"" + str_table_end + ".");
		return 5;
	}
	table_end += str_table_end.length();
	
	html_table c_table;
	if(html_table_parser::Parse(sitedata.begin() + table_begin, sitedata.begin() + table_end, m_logger, c_table)) {
		return 6;
	}
	if(!c_table.Check(m_logger, true)) {
		return 7;
	}
	c_table.RemoveSidewhites();
	STOCKS_DL_DEBUG(c_table.Print());
	m_bf_converter->DividendsFromTable(isin, c_table, dividends);
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
