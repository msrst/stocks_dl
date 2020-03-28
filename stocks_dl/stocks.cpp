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

#include <sstream>
#include "stocks.hpp"

namespace stocks_dl {

void daily_price::Print()
{
    std::cout << handelstag.AsString();
	std::cout << " " << price_open;
	std::cout << " " << price_closing;
	std::cout << " " << price_high;
	std::cout << " " << price_low;
    if(volume) {
        std::cout << " " << volume;
    }
    else {
        std::cout << " -";
    }
    std::cout << std::endl;
}

void daily_rate_nullable::AdjustPrices(double faktor)
{
	price_open *= faktor;
	price_closing *= faktor;
	price_high *= faktor;
	price_low *= faktor;
    if(volume) {
		volume /= faktor;
	}
}
void daily_rate_nullable::Print()
{
    std::cout << handelstag.AsString();
    if(price_open) {
        std::cout << " " << price_open;
    }
    else {
        std::cout << " -";
    }
    if(price_closing) {
        std::cout << " " << price_closing;
    }
    else {
        std::cout << " -";
    }
    if(price_high) {
        std::cout << " " << price_high;
    }
    else {
        std::cout << " -";
    }
    if(price_low) {
        std::cout << " " << price_low;
    }
    else {
        std::cout << " -";
    }
    if(volume) {
        std::cout << " " << volume;
    }
    else {
        std::cout << " -";
    }
    std::cout << std::endl;
}
std::string daily_rate_nullable::GetPrintingString(char separator)
{
    std::stringstream sstream;
    sstream << handelstag.AsString();
    sstream << separator;
    if(price_open) {
        sstream << price_open;
    }
    sstream << separator;
    if(price_closing) {
        sstream << price_closing;
    }
    sstream << separator;
    if(price_high) {
        sstream << price_high;
    }
    sstream << separator;
    if(price_low) {
        sstream << price_low;
    }
    sstream << separator;
    if(volume) {
        sstream << volume;
    }
    return sstream.str();
}

//---------------------------------------------------------------------------------------------------

void AdjustSplits(std::vector<daily_rate_nullable>& prices, std::vector<shares_split>& splits)
{
    if((!prices.empty()) && (!splits.empty())) {
        std::vector<shares_split>::iterator it_split = splits.end();
        it_split--;
        std::vector<daily_rate_nullable>::iterator it_price = prices.end();
        it_price--;
        for(; (it_price != prices.begin()) && ((*it_price).handelstag >= (*it_split).datum_zu); it_price--) {
        }
        if(it_price != prices.begin()) {
            double faktor = (*it_split).faktor;
            while(it_split != splits.begin()) {
                it_split--;
                for(; (it_price != prices.begin()) && ((*it_price).handelstag >= (*it_split).datum_zu); it_price--) {
                    (*it_price).AdjustPrices(faktor);
                }
                if(it_price == prices.begin()) {
                    break;
                }
                else {
                    faktor *= (*it_split).faktor;
                }
            }
            for(; it_price != prices.begin(); it_price--) {
                (*it_price).AdjustPrices(faktor);
            }
            (*it_price).AdjustPrices(faktor);
        }
    }
}

} // namespace stocks_dl
