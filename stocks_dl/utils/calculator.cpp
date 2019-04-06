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

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits.h>

#include "calculator.hpp"
#include "calc_expression.hpp"

namespace stocks_dl {
	
bool fn_calculate(std::string ausdruck_str, logger_base_ptr c_logger, double &res)
{
	typedef double Tzahl;
	expression_parser::term<Tzahl> *oberterm;
	if(!expression_parser::ParseExpression(ausdruck_str, c_logger.get(), &oberterm)) {
		bool fehler = false;
		expression_parser::term<Tzahl>::expression_value wert = oberterm->Evaluate(&fehler);
		if(fehler) {
			expression_parser::PrintForError(oberterm, c_logger.get());
		}
		else {
			expression_parser::expression_value_numbervisitor<Tzahl> c_visitor;
			boost::apply_visitor(c_visitor, wert);
			if(c_visitor.b_zahl) {
				res = c_visitor.m_zahl;
				return true;
			}
		}
		delete oberterm;
	}
	return false;
}

} // namespace stocks_dl
