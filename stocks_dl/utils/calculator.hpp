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
#ifndef STOCKS_DL_CALCULATOR_HPP_INCLUDED
#define STOCKS_DL_CALCULATOR_HPP_INCLUDED

#include "logger.hpp"

namespace stocks_dl {
	
bool fn_calculate(std::string ausdruck_str, logger_base_ptr c_logger, double &res);

} // namespace stocks_dl

#endif // STOCKS_DL_CALCULATOR_HPP_INCLUDED
