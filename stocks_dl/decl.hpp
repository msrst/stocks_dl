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

#ifndef STOCKS_DL_DECL_HPP_INCLUDED
#define STOCKS_DL_DECL_HPP_INCLUDED

#include "stocks_dl_config.h"

//#define STOCKS_DL_DEBUG_OUTPUT

#ifdef STOCKS_DL_DEBUG_OUTPUT
 #define STOCKS_DL_DEBUG(code) code
#else
 #define STOCKS_DL_DEBUG(code)
#endif

namespace stocks_dl {
	
struct daily_price;
struct daily_rate_nullable;
struct stock_share;
class mcurlwrapper;
struct dividend;

} // namespace stocks_dl

#endif // STOCKS_DL_DECL_HPP_INCLUDED
