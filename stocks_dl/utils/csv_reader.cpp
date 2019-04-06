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
#include <iostream>
#include <algorithm>

#include "utils.hpp"
#include "csv_reader.hpp"
#include "../decl.hpp"

namespace stocks_dl {
namespace csv_reader {

reader::reader(char separator, table_consumer* c_table_consumer)
{
    m_separator = separator;
    m_table_consumer = c_table_consumer;
}
reader::~reader()
{
    //
}

int reader::Parse(std::istream& instream)
{
    char zeichen;

    InitParserData();

    while(instream.get(zeichen)) {
        int err = ParseChar(zeichen);
        if(err) {
            return err;
        }
    }

    switch(status) {
     case reader_stat_line1_column:
        return 1;
        break;
     case reader_stat_value:
        if((it_column != m_columns.begin()) || (current_value.size() != 0)) { // When the file does not end with a '\n', the last line contains data
            current_line[*it_column] = current_value;
            current_value.clear();
            line_number++;
            it_column = m_columns.begin();
            if(m_table_consumer->AddLine(current_line) == false) {
                return 5;
            }
            current_line.clear();
        }
        break;
    }

    return 0;
}
std::string reader::GetError(int fehler)
{
    std::string beschreibung;
    switch(fehler) {
     case CSVERR_NO_ERROR:
        beschreibung = "successful";
        break;
     case CSVERR_FIRST_LINE_UNCOMPLETE:
        beschreibung = "no complete first line";
        break;
     case CSVERR_EMPTY_COLUMN_HEADING:
        beschreibung = "empty column heading";
        break;
     case CSVERR_DUPLICATE_COLUMN:
        beschreibung = "duplicate column";
        break;
     case CSVERR_MORE_COLUMNS_IN_ONE_ROW:
        beschreibung = "too much columns in row " + Int_To_String(line_number);
        break;
     case CSVERR_WRONG_DATA_FORMAT:
        beschreibung = "wrong data format in row " + Int_To_String(line_number); // The row was not accepted by the table_consumer
        break;
     default:
        beschreibung = "unknown error with code " + Int_To_String(fehler);
        break;
    }
    return beschreibung;
}

void reader::InitParserData()
{
    status = 0;
    line_number = 0;
}
int reader::ParseChar(char zeichen)
{
    int i1 = 0;
    i1 += 1;
    switch(status) {
     case reader_stat_line1_column:
        if(zeichen == m_separator) {
            if(current_column_heading.size() == 0) {
                return 2;
            }
            else if(std::find(m_column_headings.begin(), m_column_headings.end(), current_column_heading) != m_column_headings.end()) {
                return 3;
            }
            else {
                m_column_headings.push_back(current_column_heading);
                m_columns.push_back(m_table_consumer->GetColumnIndex(current_column_heading));
                //std::cout << m_columns.back() << " -> " << current_column_heading << std::endl;
                current_column_heading.clear();
            }
        }
        else if(zeichen == '\n') {
            if(current_column_heading.size() == 0) {
                return 2;
            }
            else if(std::find(m_column_headings.begin(), m_column_headings.end(), current_column_heading) != m_column_headings.end()) {
                return 3;
            }
            else {
                m_column_headings.push_back(current_column_heading);
                m_columns.push_back(m_table_consumer->GetColumnIndex(current_column_heading));
                //std::cout << m_columns.back() << " -> " << current_column_heading << std::endl;
                current_column_heading.clear();
            }
            status++;
            it_column = m_columns.begin();
            line_number++;
        }
        else {
            current_column_heading.push_back(zeichen);
        }
        break;
     case reader_stat_value:
        if(zeichen == m_separator) {
            current_line[*it_column] = current_value;
            it_column++;
            current_value.clear();
            if(it_column == m_columns.end()) {
                return 4;
            }
        }
        else if(zeichen == '\n') {
            current_line[*it_column] = current_value;
            current_value.clear();
            line_number++;
            it_column = m_columns.begin();
            if(m_table_consumer->AddLine(current_line) == false) {
                return 5;
            }
            current_line.clear();
        }
        else {
            current_value.push_back(zeichen);
        }
        break;
    }
    return 0;
}

} // namespace csv_reader
} // namespace stocks_dl
