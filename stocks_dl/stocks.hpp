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

#ifndef STOCKS_DL_STOCKS_HPP_INCLUDED
#define STOCKS_DL_STOCKS_HPP_INCLUDED

#include "time_day.hpp"
#include "boerse_frankfurt.hpp"
#include "utils/tnull.hpp"
#include "decl.hpp"

namespace stocks_dl {
	
struct isin_assignment
{
	std::string name;
	std::string isin;
};
struct stock_share
{
    unsigned int id; // freely usable id, p. e. for databases etc.
    std::string symbol_name;
    std::string symbol_boerse;
    std::string name;
    std::string isin;
    
    stock_share(unsigned int id = 0) {
		this->id = id;
	}
	stock_share(const isin_assignment &c_isin_assignment) {
		name = c_isin_assignment.name;
		isin = c_isin_assignment.isin;
	}
	
	std::string YSymbol() const {
		return symbol_name + '.' + symbol_boerse;
	}
	bool FromYSymbol(std::string symbol) {
		std::vector<std::string> parts = str_split(symbol, ".");
		symbol_name = parts[0];
		if(parts.size() == 2) {
			symbol_boerse = parts[1];
		}
		else if(parts.size() > 2) { // symbol is incorrect
			return false;
		}
		return true;
	}
	std::string SimpleName() const {
		if(str_ends_with(name, " AG")) {
			return name.substr(0, name.length() - 3);
		}
		if(str_ends_with(name, " Aktiengesellschaft")) {
			return name.substr(0, name.length() - 19);
		}
		else if(str_ends_with(name, " S.A")) {
			return name.substr(0, name.length() - 4);
		}
		else if(str_ends_with(name, " S.A.")) {
			return name.substr(0, name.length() - 5);
		}
		else if(str_ends_with(name, " SA")) {
			return name.substr(0, name.length() - 3);
		}
		else if(str_ends_with(name, " SE")) {
			return name.substr(0, name.length() - 3);
		}
		else if(str_ends_with(name, " Ltd.")) {
			return name.substr(0, name.length() - 5);
		}
		else if(str_ends_with(name, " Ltd")) {
			return name.substr(0, name.length() - 4);
		}
		else if(str_ends_with(name, " AG & Co. KGaA")) {
			return name.substr(0, name.length() - 14);
		}
		else if(str_ends_with(name, " & Co")) {
			return name.substr(0, name.length() - 5);
		}
		else if(str_ends_with(name, " & CO. KGAA")) {
			return name.substr(0, name.length() - 11);
		}
		else {
			return name;
		}
	}
	
};

struct daily_price // price & volume of a day
{
    time_day handelstag;
    double price_open;
    double price_closing;
    double price_high;
    double price_low;
    tnull_double_adv volume;
    
    void Print();
};
class daily_rate_nullable
{
public:
    time_day handelstag;
    tnull_double_adv price_open;
    tnull_double_adv price_closing;
    tnull_double_adv price_high;
    tnull_double_adv price_low;
    tnull_double_adv volume;

    void AdjustPrices(double factor); // Also adjusts the volume
    void Print();
    std::string GetPrintingString(char separator);
};
struct shares_split
{
    time_day datum_zu;
    double faktor;
};
struct dividend
{
	time_day date;
	double dividend;
};

void AdjustSplits(std::vector<daily_rate_nullable>& prices, std::vector<shares_split>& splits); // Changes 
													// the prices vector. For this, prices[.]->AdjustPrices(.) is used.
                                            
} // namespace stocks_dl

#endif // STOCKS_DL_STOCKS_HPP_INCLUDED
