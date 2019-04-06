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

#ifndef STOCKS_DL_MINI_XML_DECL_HPP_INCLUDED
#define STOCKS_DL_MINI_XML_DECL_HPP_INCLUDED

#include <boost/variant/recursive_variant.hpp>
#include "../decl.hpp"

namespace stocks_dl {

struct mini_xml;

typedef
    boost::variant<
        boost::recursive_wrapper<mini_xml>
      , std::string
    >
mini_xml_node;

struct mini_xml_nodeparam
{
    std::string name;
    std::string value;
};

struct mini_xml
{
    std::string name;                           // tag name
    std::vector<mini_xml_nodeparam> parameter;
    std::vector<mini_xml_node> children;        // children
};

} // namespace stocks_dl

#endif // STOCKS_DL_MINI_XML_DECL_HPP_INCLUDED
