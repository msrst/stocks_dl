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

#include "boerse_frankfurt_parser.hpp"
#include "utils/utils.hpp"
#include "stocks.hpp"
#include "curlwrapper.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace pt = boost::property_tree;

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

bf_converter::bf_converter(logger_base_ptr c_logger):
	currency_balance_sheet_field(		 "Bilanzierungsw\xc3\xa4hrung")
{
	m_logger = c_logger;

	balance_sheet_fields.emplace_back("assetsCapitalTotal");
	balance_sheet_fields.emplace_back("assetsCurrentTotal");
	balance_sheet_fields.emplace_back("assetsTotal");
	balance_sheet_fields.emplace_back("assetsTotalPerShare");
	balance_sheet_fields.emplace_back("bookvaluePerShare");
	balance_sheet_fields.emplace_back("cashflowPerShare");
	balance_sheet_fields.emplace_back("currencyCode");
	balance_sheet_fields.emplace_back("debtEquityRatio", true);
	balance_sheet_fields.emplace_back("deptRatio", true);
	balance_sheet_fields.emplace_back("dividendPayment");
	balance_sheet_fields.emplace_back("dividendPerShare");
	balance_sheet_fields.emplace_back("dividendPerShareExtra");
	balance_sheet_fields.emplace_back("dividendReturnRatio", true);
	balance_sheet_fields.emplace_back("earningsPerShareBasic");
	balance_sheet_fields.emplace_back("earningsPerShareDiluted");
	balance_sheet_fields.emplace_back("earningsPerShareGross");
	balance_sheet_fields.emplace_back("employees");
	balance_sheet_fields.emplace_back("equityRatio", true);
	balance_sheet_fields.emplace_back("equityReturnRatio", true);
	balance_sheet_fields.emplace_back("equityTotal");
	balance_sheet_fields.emplace_back("expensesPerEmployee");
	balance_sheet_fields.emplace_back("incomeAfterTax");
	balance_sheet_fields.emplace_back("incomeBeforeTax");
	balance_sheet_fields.emplace_back("incomeNet");
	balance_sheet_fields.emplace_back("incomeNetPerEmployee", true);
	balance_sheet_fields.emplace_back("incomeNetRatio", true);
	balance_sheet_fields.emplace_back("incomeOperating");
	balance_sheet_fields.emplace_back("intensityOfLaborRatio", true);
	balance_sheet_fields.emplace_back("liabilitiesAndEquityTotal");
	balance_sheet_fields.emplace_back("liabilitiesCurrentTotal");
	balance_sheet_fields.emplace_back("liabilitiesLongtermTotal");
	balance_sheet_fields.emplace_back("liabilitiesTotal");
	balance_sheet_fields.emplace_back("minorityInterest");
	balance_sheet_fields.emplace_back("outstandingClassShares");
	balance_sheet_fields.emplace_back("outstandingShares");
	balance_sheet_fields.emplace_back("personnelExpenses");
	balance_sheet_fields.emplace_back("priceBookRatio", true);
	balance_sheet_fields.emplace_back("priceCashFlowRatio", true);
	balance_sheet_fields.emplace_back("priceEarningsRatio", true);
	balance_sheet_fields.emplace_back("priceSalesRatio", true);
	balance_sheet_fields.emplace_back("profitGross");
	balance_sheet_fields.emplace_back("profitGrossPerEmployee");
	balance_sheet_fields.emplace_back("returnOnAssetsRatio", true);
	balance_sheet_fields.emplace_back("returnOnInvestment");
	balance_sheet_fields.emplace_back("salesPerEmployee");
	balance_sheet_fields.emplace_back("salesPerShare");
	balance_sheet_fields.emplace_back("salesRatio", true);
	balance_sheet_fields.emplace_back("salesRevenue");
	balance_sheet_fields.emplace_back("subscribedCapital");
	balance_sheet_fields.emplace_back("taxesOnIncome");
	balance_sheet_fields.emplace_back("workingCapital");

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

int bf_converter::BalanceSheetFromJson(long int stock_id, const std::string &json, std::vector<balance_sheet> &balance_sheets)
{
	pt::ptree tree;

	std::vector<balance_sheet> res;
	std::stringstream sstream(json);
	try {
		pt::read_json(sstream, tree);

		for(const pt::ptree::value_type &sheet_json : tree.get_child("data")) {
			balance_sheet new_sheet;
			new_sheet.stock_id = stock_id;
			new_sheet.year = sheet_json.second.get<int>("year");
			new_sheet.currency = sheet_json.second.get<std::string>("currencyCode");

			for(const pt::ptree::value_type &field_json : sheet_json.second) {
				if((field_json.first == "year") || (field_json.first == "currencyCode")) {
					continue;
				}

				std::map<std::string, unsigned int>::iterator it_num =
					bf_to_num.find(field_json.first);
				if(it_num == bf_to_num.end()) {
					m_logger->LogWarning("Boerse Frankfurt: could not use balance sheet value " +
						field_json.first + ".");
				}
				else {
					boost::optional<double> value =
							field_json.second.get_value_optional<double>();
					if(value) {
						new_sheet.data[it_num->second] = *value;
					}
					else {
						// this just happens to often so don't pollute the command line
						//m_logger->LogInformation("Boerse Frankfurt: could not convert balance sheet value for " +
						//	field_names[it_num->second] +
						//		" to double. This can happen if it is null, for example.");
					}
				}
			}
			res.push_back(new_sheet);
		}
	}
	catch(boost::property_tree::ptree_error &e) {
		m_logger->LogError("Error when parsing share data json from BoerseFrankfurt: " + (std::string)e.what() + ".");
		return 1;
	}

	balance_sheets.insert(balance_sheets.end(), res.begin(), res.end());
	return 0;
}
int bf_converter::PricesFromJson(const std::string &json, std::vector<daily_rate_nullable> &prices)
{
	prices.clear();

    pt::ptree tree;

	std::stringstream sstream(json);
	try {
		pt::read_json(sstream, tree);

		int declaredCount = tree.get<int>("totalCount");
		for(const pt::ptree::value_type &price_json : tree.get_child("data")) {
			daily_rate_nullable new_price;
			bool ok;
			new_price.handelstag = TimeDayFromString(price_json.second.get<std::string>("date"), &ok);
			if(!ok) {
				m_logger->LogError("Could not convert date " + price_json.second.get<std::string>("date") + ".");
				return 1;
			}
			new_price.price_low = price_json.second.get_optional<double>("low");
			new_price.price_high = price_json.second.get_optional<double>("high");
			new_price.price_closing = price_json.second.get_optional<double>("close");
			new_price.price_open = price_json.second.get_optional<double>("open");
			new_price.volume = price_json.second.get_optional<double>("turnoverPieces"); // note that there is also a field "turnoverEuro"
			prices.push_back(new_price);
		}
		if(prices.size() != declaredCount) {
			m_logger->LogError("declared count of prices " + std::to_string(declaredCount) +
				" != actual count " + std::to_string(prices.size()) + ".");
			return 3;
		}
	}
	catch(boost::property_tree::ptree_error &e) {
		m_logger->LogError("Error when parsing json from BoerseFrankfurt: " + (std::string)e.what() + ".");
		return 2;
	}
	return 0;
}
int bf_converter::DividendsFromJson(const std::string &isin_check, const std::string &json, std::vector<dividend> &dividends)
{
	dividends.clear();

    pt::ptree tree;

	std::stringstream sstream(json);
	try {
		pt::read_json(sstream, tree);

		for(const pt::ptree::value_type &price_json : tree.get_child("data")) {
			dividend new_dividend;
			bool ok;
			new_dividend.date = TimeDayFromString(price_json.second.get<std::string>("dividendLastPayment"), &ok);
			if(!ok) {
				m_logger->LogError("Could not convert dividend date " + price_json.second.get<std::string>("dividendLastPayment") + ".");
				return 1;
			}
			new_dividend.dividend = price_json.second.get<double>("dividendValue");
			new_dividend.currency = price_json.second.get<std::string>("dividendCurrency");
			std::string dividend_isin = price_json.second.get<std::string>("dividendIsin");
			if(dividend_isin != isin_check) {
				m_logger->LogWarning("Ignored divided with invalid isin " + dividend_isin + " for share with isin " + isin_check + ".");
			}
			else {
				dividends.push_back(new_dividend);
			}
		}
	}
	catch(boost::property_tree::ptree_error &e) {
		m_logger->LogError("Error when parsing json from BoerseFrankfurt with isin " + isin_check + "dividend: " + (std::string)e.what() + ".");
		return 2;
	}
	return 0;
}

bool bf_converter::GetBFName_ViaSearch(const std::string &isin, mcurlwrapper &mcurl, std::string &bfname)
{
	if(!mcurl.IsOk()) {
		m_logger->LogError("Curl not initialised!");
		return false;
	}

	const std::string getsite_url = "https://api.boerse-frankfurt.de/v1/global_search/limitedsearch/de?searchTerms=" + isin;
	std::string sitedata;
	if(mcurl.GetSite(getsite_url, sitedata, true, CalcTokens(getsite_url))) {
		return false;
	}

	const std::string begin = "[[{\"isin\":\"" + isin + "\",\"slug\":\"";
	if(!str_begins_with(sitedata, begin)) {
		m_logger->LogError("Query at " + getsite_url + " does not begin with " + begin + ".");
		m_logger->LogInformation("Sitedata was: " + sitedata.substr(0, std::min((std::size_t)200, sitedata.size())));
		return false;
	}
	size_t bfname_end = sitedata.find('"', begin.length());
	if(bfname_end == std::string::npos) {
		m_logger->LogError("BoerseFrankfurt-name did not end with double quotation (\").");
		return false;
	}
	size_t bfname_length = bfname_end - begin.length();
	if((bfname_length <= 0) || (bfname_length > 50)) {
		m_logger->LogError("Boerse Frankfurt returned name with invalid length " + std::to_string(bfname_length) + ".");
		return false;
	}
	bfname = sitedata.substr(begin.length(), bfname_length);
	STOCKS_DL_DEBUG(std::cout << "Boerse Frankfurt name = " + bfname + "." << std::endl;);
	return true;
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

} // namespace boerse_frankfurt
} // namespace stocks_dl
