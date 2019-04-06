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

#include "utils.hpp"
#include "mini_xml.hpp"

#include <boost/bind.hpp>

namespace stocks_dl {
	
xml_value_visitor::xml_value_visitor()
{
    value_stat = 0;
}

void xml_value_visitor::operator()(mini_xml& xmlnode)
{
    value_stat = 2;
}
void xml_value_visitor::operator()(std::string& str)
{
    if(value_stat == 0) {
        m_value = str;
        value_stat = 1;
    }
    else if(value_stat == 1) {
        value_stat = 2;
    }
}
bool xml_value_visitor::GetValue(std::string* value)
{
    if(value_stat == 2) {
        return false;
    }
    else {
        *value = m_value;
        return true;
    }
}

//---------------------------------------------------------------------------------------------------

void xml_htmltext_visitor::operator()(mini_xml& xmlnode)
{
    xml_htmltext_visitor visitor(m_value);
    std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
}

} // namespace stocks_dl
