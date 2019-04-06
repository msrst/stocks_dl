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
#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include <string>
#include <boost/shared_ptr.hpp>
#include "../decl.hpp"

class logger_base
{
public:
    virtual ~logger_base() {}

    virtual void LogError(std::string fehler) = 0;
    virtual void LogWarning(std::string warnung) = 0;
    virtual void LogInformation(std::string meldung) = 0;
};

typedef boost::shared_ptr<logger_base> logger_base_ptr;

#endif // LOGGER_HPP_INCLUDED
