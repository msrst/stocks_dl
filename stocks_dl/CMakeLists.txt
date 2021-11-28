# Copyright (C) 2019 Matthias Rosenthal
#
# This file is part of stocks_dl.
#
# Stocks_dl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# Stocks_dl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with stocks_dl. If not, see <http://www.gnu.org/licenses/>
######################################################################

cmake_minimum_required (VERSION 3.0.2)
project (stocks_dl_lib)

# If you do not want to use the default boost lib or if you do not have
# installed it, you can use your own with cmake option -DBOOST_ROOT=/your/boost/root
find_package(Boost COMPONENTS system date_time)
find_package(CURL)

# Can be set via cmake option -DSTOCKS_DL_DEBUG_OUTPUT=ON
# (in cmake-gui, this is more intuitive)
# Options are stored in the cmake cache. This means that when setting
# the option, it is kept until the setting is unset.
option(STOCKS_DL_DEBUG_OUTPUT "Outputs some internal information" OFF) # OFF means default of

configure_file (
	"stocks_dl_config.h.in"
	"stocks_dl_config.h"
)

add_library(stocks_dl boerse_frankfurt.cpp boerse_frankfurt_sheet.cpp boerse_frankfurt_parser.cpp
    curlwrapper.cpp dividends.cpp stocks.cpp time_day.cpp yahoodownloader.cpp
    utils/logger.cpp utils/calculator.cpp utils/csv_reader.cpp utils/utils.cpp)
# include ${CMAKE_CURRENT_BINARY_DIR} (resolved to build/stocks_dl or something like that) to
# include the generated config file stocks_dl_config.h
target_include_directories(stocks_dl PUBLIC ${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(stocks_dl ${Boost_LIBRARIES} pthread ssl crypto ${CURL_LIBRARIES})
