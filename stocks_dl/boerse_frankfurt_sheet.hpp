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

#ifndef BOERSE_FRANKFURT_SHEET_HPP_INCLUDED
#define BOERSE_FRANKFURT_SHEET_HPP_INCLUDED

#include "decl.hpp"
#include "utils/tnull.hpp"
#include <string>

namespace stocks_dl {
namespace boerse_frankfurt {

const int BALANCE_SHEET_FIELD_COUNT = 51;
const std::string FRANKFURT_STOCK_EXCHANGE = "XFRA";
const std::string XETRA = "XETR";
	
// This fields are used for balance_sheet.data, p. e. my_balance_sheet.data[earning_per_share]
// equals to the earning per share.
// The fields also correspond to those of the english version of boerse-frankfurt.de:
// p. e. http://en.boerse-frankfurt.de/stock/keydata/Siemens-share/FSE#Key%20Data
enum fields {
	bffield_assets_capital_total = 0,
	bffield_assets_current_total,
	bffield_assets_total,
	bffield_assets_total_per_share,
	bffield_book_value_per_share,
	bffield_cashflow_per_share,
	bffield_currency_code,
	bffield_debt_equity_ratio,
	bffield_dept_ratio,
	bffield_dividend_payment,
	bffield_dividend_per_share,
	bffield_dividend_per_share_extra,
	bffield_dividend_return_ratio,
	bffield_earnings_per_share_basic,
	bffield_earnings_per_share_diluted,
	bffield_earnings_per_share_gross,
	bffield_employees,
	bffield_equity_ratio,
	bffield_equity_return_ratio,
	bffield_equity_total,
	bffield_expenses_per_employee,
	bffield_income_after_tax,
	bffield_income_before_tax,
	bffield_income_net,
	bffield_income_net_per_employee,
	bffield_income_net_ratio,
	bffield_income_operating,
	bffield_intensity_of_labor_ratio,
	bffield_liabilities_and_equity_total,
	bffield_liabilities_current_total,
	bffield_liabilities_longterm_total,
	bffield_liabilities_total,
	bffield_minority_interest,
	bffield_outstanding_class_shares,
	bffield_outstanding_shares,
	bffield_personnel_expenses,
	bffield_price_book_ratio,
	bffield_price_cash_flow_ratio,
	bffield_price_earnings_ratio,
	bffield_price_sales_ratio,
	bffield_profit_gross,
	bffield_profit_gross_per_employee,
	bffield_return_on_assets_ratio,
	bffield_return_on_investment,
	bffield_sales_per_employee,
	bffield_sales_per_share,
	bffield_sales_ratio,
	bffield_sales_revenue,
	bffield_subscribed_capital,
	bffield_taxes_on_income,
	bffield_working_capital
};
// I generated this with excel from an embedded json on the english version of boerse-frankfurt
const std::string field_names[BALANCE_SHEET_FIELD_COUNT] = {
	"Fixed assets",
    "Total current assets",
    "Assets",
    "Total assets per share",
    "Book value per share",
    "Cashflow per share",
    "Financial reporting currency",
    "Debt Quity ratio",
    "Debt ratio",
    "Dividend",
    "Dividend per share",
    "Extra dividend per Share",
    "Dividend yield",
    "Earnings per share (basic)",
    "Earnings per share (diluted)",
    "Earnings per share (gross)",
    "Number of employees",
    "Equity Ratio",
    "Return on equity",
    "Total equity",
    "Cost per employee",
    "Result after taxes",
    "EBT",
    "Net income / loss",
    "Earnings per employee",
    "Earnings yield",
    "EBIT",
    "Work intensity",
    "Liabilities",
    "Short-term debt",
    "Long-term debt",
    "Total debt",
    "Minority share",
    "Class shares in circulation",
    "Shares in circulation",
    "Personnel expenses",
    "P/B Ratio",
    "KCV",
    "P/E",
    "KUV",
    "Gross profit",
    "Gross profit per employee",
    "Total return on Assets",
    "ROI",
    "Sales per employee",
    "Sales per share",
    "Return on sales",
    "Sales",
    "Subscribed capital",
    "Income tax payments",
    "Working Capital"
};
// don't worry about errors - I generated this in excel from the BF
// descriptions (embedded json in e. g. in 
// https://www.boerse-frankfurt.de/aktie/siemens-ag main document)
const bool printed_mio[BALANCE_SHEET_FIELD_COUNT] = {
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	true,
	true,
	false,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true
};
const size_t field_names_max_len = 38;
const std::string field_names_german[BALANCE_SHEET_FIELD_COUNT] = {
	"Anlagevermögen",
	"Summe Umlaufvermögen",
	"Aktiva",
	"Bilanzsumme je Aktie",
	"Buchwert je Aktie",
	"Cashflow je Aktie",
	"Bilanzierungswährung",
	"Verschuldungsgrad",
	"Fremdkapitalquote",
	"Dividendenausschüttung",
	"Dividende je Aktie",
	"Extra-Dividende je Aktie",
	"Dividendenrendite",
	"Ergebnis je Aktie (unverwässert)",
	"Ergebnis je Aktie (verwässert)",
	"Ergebnis je Aktie (brutto)",
	"Anzahl Mitarbeiter",
	"Eigenkapitalquote",
	"Eigenkapitalrendite",
	"Summe Eigenkapital",
	"Aufwand je Mitarbeiter",
	"Ergebnis n. Steuern",
	"EBT",
	"Jahresüberschuss/-fehlbetrag",
	"Gewinn je Mitarbeiter",
	"Gewinnrendite",
	"EBIT",
	"Arbeitsintensität",
	"Passiva",
	"kurzfristiges Fremdkapital",
	"langfristiges Fremdkapital",
	"Summe Fremdkapital",
	"Minderheitenanteil",
	"Vorzugsaktien im Umlauf",
	"Aktien im Umlauf",
	"Personalaufwand",
	"KBV",
	"KCV",
	"KGV",
	"KUV",
	"Bruttoergebnis vom Umsatz",
	"Bruttoergebnis je Mitarbeiter",
	"Gesamtkapitalrendite",
	"ROI",
	"Umsatz je Mitarbeiter",
	"Umsatz je Aktie",
	"Umsatzrendite",
	"Umsatz",
	"Gezeichnetes Kapital",
	"Einkommens- und Ertragssteuerzahlungen",
	"Working Capital"
};
const size_t field_names_german_max_len = 38;

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

} // namespace boerse_frankfurt
} // namespace stocks_dl
 
#endif // BOERSE_FRANKFURT_SHEET_HPP_INCLUDED