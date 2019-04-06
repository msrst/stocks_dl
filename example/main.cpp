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

// cmake . -DBOOST_ROOT=/home/matthias/cpp_ubuntu/boost_1_67_0ub -DCMAKE_BUILD_TYPE=Debug
// usage examples with gdb:
// gdb example1
// r y_dividends BAS.DE BASF
// r bf_dividends DE0007236101
// r bf_share_data DE0007236101
// r bf_prices DE0007236101 28.1.2018_01.03.2019

#include "stocks_dl/boerse_frankfurt.hpp"
#include "stocks_dl/yahoodownloader.hpp"
#include "stocks_dl/utils/logger.h"

using namespace stocks_dl;

void PrintHelp();

int main(int argc, char *argv[])
{
    boost::shared_ptr<stocks_dl::logger::logger> c_logger = boost::make_shared<stocks_dl::logger::logger>();
    c_logger->AddLogDisplay("logfile.txt");
    boerse_frankfurt::bf_converter bf_converter(c_logger);
    
    if(argc == 4) {
		if(argv[1] == std::string("y_dividends")) {
			yahoodownloader ydl(c_logger);
			stock_share c_share;
			if(c_share.FromYSymbol(argv[2])) {
				c_share.name = argv[3];
				
				time_day trading_day_min(01, 01, 1970);
				time_day trading_day_max(01, 03, 2019);
				std::vector<dividend> dividends;
				if(ydl.Download(c_share, c_share.YSymbol(), trading_day_min, trading_day_max, dividends)) {
					std::cout << "Error: Dividends could not be downloaded." << std::endl;
				}
				else {
					for(dividend &c_dividend : dividends) {
						std::cout << c_dividend.date.AsString() << ": " << c_dividend.dividend << std::endl;
					}
				}
			}
			else {
				std::cout << "Error: symbol " << c_share.YSymbol() << " is misformatted." << std::endl;
			}
		}
		else if(argv[1] == std::string("bf_prices")) {
			boerse_frankfurt::price_downloader c_price_downloader(&bf_converter, c_logger);
			if(!c_price_downloader.Init()) {
				std::vector<daily_price> prices;
				if(c_price_downloader.Download(argv[2], argv[3], prices)) {
					std::cout << "Error: Prices could not be downloaded." << std::endl;
				}
				else {
					std::cout << "date       open close high low volume" << std::endl;
					for(daily_price & price : prices) {
						price.Print();
					}
				}
			}
		}
		else {
			PrintHelp();
		}
	}
	else if(argc == 3) {
		if(argv[1] == std::string("bf_dividends")) {
			boerse_frankfurt::share_downloader c_share_downloader(&bf_converter, c_logger);
			if(!c_share_downloader.Init()) {					
				std::vector<dividend> dividends;
				if(c_share_downloader.DownloadDividends(argv[2], dividends)) {
					std::cout << "Error: Dividends could not be downloaded." << std::endl;
				}
				else {
					for(dividend &c_dividend : dividends) {
						std::cout << c_dividend.date.AsString() << ": " << c_dividend.dividend << std::endl;
					}
				}
			}
		}
		else if(argv[1] == std::string("bf_share_data")) {
			boerse_frankfurt::share_downloader c_share_downloader(&bf_converter, c_logger);
			if(!c_share_downloader.Init()) {
				std::vector<boerse_frankfurt::balance_sheet> balance_sheets;
				std::string wkn, symbol_name;
				
				if(c_share_downloader.Download(0, argv[2], balance_sheets, wkn, symbol_name))  {
					std::cout << "Error: Share data could not be downloaded." << std::endl;
				}
				else {
					std::cout << "Symbol name: " << symbol_name << ", WKN: " << wkn << std::endl;
					for(boerse_frankfurt::balance_sheet &c_balance_sheet : balance_sheets) {
						std::cout << "-----------------------------------------" << std::endl;
						c_balance_sheet.Print();
					}
				}
			}
		}
		else {
			PrintHelp();
		}
	}
	else {
		PrintHelp();
	}
    
    return 0;
}

void PrintHelp()
{
	std::cout << "\
usage:\n\
example1 y_dividends <ysymbol> <name>\n\
  downloads dividends from finance.yahoo.com\n\
  ysymbol: p. e. BAS.DE for BASF shares (where BAS is the frankfurt\n\
     stock exchange symbol for BASF)\n\
  name: name of the share, p. e. BASF - only for verification, if the\n\
      result from yahoo does not contain the name, it is dropped\n\
example1 bf_prices <isin> <date_span>\n\
  isin: the ISIN, p. e. DE0007236101 for Siemens AG,\n\
      DE000KC01000 for Kl\xc3\xb6""ckner & Co SE\n\
  date_span: p. e. 28.1.2004_01.03.2019\n\
example1 bf_dividends <isin>\n\
  downloads dividends from www.boerse-frankfurt.de\n\
example1 bf_share_data <isin>\n\
  downloads symbol, wkn and balance sheets from\n\
      www.boerse-frankfurt.de\n\
usage examples:\n\
  ./example1 y_dividends BAS.DE BASF\n\
  ./example1 bf_dividends DE0007236101\n\
  ./example1 bf_share_data DE0007236101\n\
  ./example1 bf_prices DE0007236101 28.1.2018_01.03.2019" << std::endl;
}
