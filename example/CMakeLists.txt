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
project (stocks_dl_example)

find_package(Boost COMPONENTS system signals)
find_package(CURL)

# if set, prints the command lines of the compilation steps
#set(CMAKE_VERBOSE_MAKEFILE TRUE)

#if(Boost_FOUND)
  add_executable(example1 main.cpp)
  # ../ is for stocks_dl
  target_include_directories(example1 PUBLIC ${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ../)
  target_link_libraries(example1 ${Boost_LIBRARIES} pthread ssl crypto ${CURL_LIBRARIES} stocks_dl)
#endif()
