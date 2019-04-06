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

#include <fstream>
#include <sstream>

#include "utils/utils.hpp"
#include "table_parser.hpp"
#include "utils/mini_xml.hpp"

#include <boost/bind.hpp>
#include "utils/utils.hpp"

namespace stocks_dl {

bool html_table::Check(logger_base_ptr c_logger, bool adjust_spaltenheading) 
{
	size_t r_heading_row_size = heading_row.size();
	if(zeilen.size() != heading_column.size()) { // Vom table_parser werden normalerweise immer Zeilen hinzugefügt, auch wenn keine angegeben ist.
		c_logger->LogError("Table invalid (" + Int_To_String(r_heading_row_size) + '/'
				+ Int_To_String(zeilen.size()) + " rows/" + Int_To_String(heading_column.size())
			    + "): Heading-columns has " + Int_To_String(heading_column.size()) + " row, but there are "
			    + Int_To_String(zeilen.size()) + " rows.");
		return false;
	}
	if(adjust_spaltenheading) {
		if(!zeilen.empty()) {
			unsigned int zeilen_laenge = zeilen[0].size();
			for(unsigned int i1 = 1; i1 < zeilen.size(); i1++) {
				if(zeilen[i1].size() != zeilen_laenge) {
					c_logger->LogError("Table invalid (" + Int_To_String(heading_row.size()) + '/'
							+ Int_To_String(zeilen.size()) + '/' + Int_To_String(heading_column.size())
						    + "): Row " + Int_To_String(i1) + " has " + Int_To_String(zeilen[i1].size())
						    + " rows, but the first row has " + Int_To_String(zeilen_laenge) + ".");
					return false;
				}
			}
			if(r_heading_row_size == zeilen_laenge - 1) {
				for(unsigned int i1 = 0; i1 < heading_column.size(); i1++) {
					if(!heading_column[i1].empty()) {
						c_logger->LogError("Table invalid (" + Int_To_String(heading_row.size()) + '/'
								+ Int_To_String(zeilen.size()) + '/' + Int_To_String(heading_column.size())
								+ "): Every row has " + Int_To_String(zeilen_laenge) + " cells, but the row heading "
								+ Int_To_String(i1) + " is not empty (\"" + heading_column[i1] + "\").");
						return false;
					}
				}
				for(unsigned int izeile = 0; izeile < zeilen.size(); izeile++) {
					heading_column[izeile] = zeilen[izeile][0];
					zeilen[izeile].erase(zeilen[izeile].begin());
				}
			}
			else if(r_heading_row_size != zeilen_laenge) {
				c_logger->LogError("Table invalid (" + Int_To_String(heading_row.size()) + '/'
						+ Int_To_String(zeilen.size()) + '/' + Int_To_String(heading_column.size())
					    + "): Every row has " + Int_To_String(zeilen_laenge) + "cells, but the heading row has "
					    + Int_To_String(heading_row.size()) + " cells.");
				return false;
			}
		}
	}			
	else {
		for(unsigned int i1 = 0; i1 < zeilen.size(); i1++) {
			if(zeilen[i1].size() != r_heading_row_size) {
				c_logger->LogError("Table invalid (" + Int_To_String(heading_row.size()) + '/'
						+ Int_To_String(zeilen.size()) + '/' + Int_To_String(heading_column.size())
						+"): Row " + Int_To_String(i1) + " has " + Int_To_String(zeilen[i1].size()) + " cells.");
				return false;
			}
		}
	}
	return true;
}
static size_t max_length(const std::vector<std::string> &strings, std::string weiterer_string = std::string())
{
	size_t max = UTF8Laenge(weiterer_string);
	for(const std::string &str : strings) {
		if(UTF8Laenge(str) > max) {
			max = UTF8Laenge(str);
		}
	}
	return max;
}
void html_table::Print()
{
	size_t heading_column_max = max_length(heading_column, heading_heading);
	std::vector<size_t> spalten_max(heading_row.size());
	for(unsigned int i1 = 0; i1 < heading_row.size(); i1++) {
		spalten_max[i1] = UTF8Laenge(heading_row[i1]);
		for(unsigned int i2 = 0; i2 < zeilen.size(); i2++) {
			if(UTF8Laenge(zeilen[i2][i1]) > spalten_max[i1]) {
				spalten_max[i1] = UTF8Laenge(zeilen[i2][i1]);
			}
		}
	}
	std::cout << heading_heading << std::string(heading_column_max - heading_heading.size(), ' ');
	for(unsigned int i2 = 0; i2 < heading_row.size(); i2++) {
		std::cout << '|' << heading_row[i2] << std::string(spalten_max[i2] - UTF8Laenge(heading_row[i2]), ' ');
	}
	std::cout << std::endl;
	for(unsigned int i1 = 0; i1 < zeilen.size(); i1++) {
		std::cout << heading_column[i1] << std::string(heading_column_max - UTF8Laenge(heading_column[i1]), ' ');
		for(unsigned int i2 = 0; i2 < zeilen[i1].size(); i2++) {
			std::cout << '|' << zeilen[i1][i2] << std::string(spalten_max[i2] - UTF8Laenge(zeilen[i1][i2]), ' ');
		}
		std::cout << std::endl;
	}
}
void html_table::RemoveSidewhites()
{
	heading_heading = str_remove_sidewhites(heading_heading);
	for(unsigned int ispalte = 0; ispalte < heading_row.size(); ispalte++) {
		heading_row[ispalte] = str_remove_sidewhites(heading_row[ispalte]);
	}
	for(unsigned int izeile = 0; izeile < zeilen.size(); izeile++) {
		heading_column[izeile] = str_remove_sidewhites(heading_column[izeile]);
		for(unsigned int ispalte = 0; ispalte < heading_row.size(); ispalte++) {
			zeilen[izeile][ispalte] = str_remove_sidewhites(zeilen[izeile][ispalte]);
		}
	}
}

//----------------------------------------------------------------------------------------------

namespace html_table_parser {

//----------------------------------------------------------------------------------------------

void body_tr_visitor::operator()(mini_xml &xmlnode)
{
	if(xmlnode.name == "th") {
		if(m_parseconfig->b_inner_html_allowed) {
			xml_htmltext_visitor visitor(m_heading);
			std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
			m_def_counts.th++;
			if(!m_neue_zeile->empty()) {
				m_logger->LogError("Table-Parser: th read after tr in the tbody.");
				*m_b_ok = false;
			}
		}
		else {
			xml_value_visitor visitor;
			std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
			if(!visitor.GetValue(m_heading)) {
				m_logger->LogError("Table-Parser: Error while reading the th tag of the tbody-tr.");
				*m_b_ok = false;
			}
			else {
				m_def_counts.th++;
				if(!m_neue_zeile->empty()) {
					m_logger->LogError("Table-Parser: th read after tr in the tbody.");
					*m_b_ok = false;
				}
			}
		}
	}
	else if(xmlnode.name == "td") {
		std::string neue_zelle;
		if(m_parseconfig->b_inner_html_allowed) {
			xml_htmltext_visitor visitor(&neue_zelle);
			std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
			m_neue_zeile->push_back(neue_zelle);
		}
		else {
			xml_value_visitor visitor;
			std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
			if(!visitor.GetValue(&neue_zelle)) {
				m_logger->LogError("Table-Parser: Error while reading a td tag of a tbody-tr.");
				*m_b_ok = false;
			}
			else {
				m_neue_zeile->push_back(neue_zelle);
			}
		}
	}
	else {
		m_logger->LogInformation("Table-Parser: ignoring tbody-tr-node \"" + xmlnode.name + "\".");
	}
}

//----------------------------------------------------------------------------------------------

void head_tr_visitor::operator()(mini_xml &xmlnode)
{
	if(xmlnode.name == "th") {
		std::string cell;
		if(m_parseconfig->b_inner_html_allowed) {
			xml_htmltext_visitor visitor(&cell);
			std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
			if(m_def_counts.heading_heading == 0) {
				*m_heading = cell;
				m_def_counts.heading_heading++;
			}
			else {
				m_neue_zeile->push_back(cell);
			}
		}
		else {
			xml_value_visitor visitor;
			std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
			if(!visitor.GetValue(&cell)) {
				m_logger->LogError("Table-Parser: Error while reading the th tag in the thead-tr.");
				*m_b_ok = false;
			}
			else {
				if(m_def_counts.heading_heading == 0) {
					*m_heading = cell;
					m_def_counts.heading_heading++;
				}
				else {
					m_neue_zeile->push_back(cell);
				}
			}
		}
	}
	else {
		m_logger->LogInformation("Table-Parser: Ignoring thead-tr-node \"" + xmlnode.name + "\".");
	}
}

//----------------------------------------------------------------------------------------------

void tbody_visitor::operator()(mini_xml &xmlnode)
{
	if(xmlnode.name == "tr") {
		std::string heading;
		std::vector<std::string> neue_zeile;
		body_tr_visitor visitor(&heading, &neue_zeile, m_logger, m_b_ok, m_parseconfig);
		std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
		visitor.CheckDefCounts();
		m_table->heading_column.push_back(heading);
		m_table->zeilen.push_back(neue_zeile);
	}
	else {
		m_logger->LogInformation("Table-Parser: Ignoring tbody-node \"" + xmlnode.name + "\".");
	}
}

//----------------------------------------------------------------------------------------------

void thead_visitor::operator()(mini_xml &xmlnode)
{
	if(xmlnode.name == "tr") {
		head_tr_visitor visitor(&(m_table->heading_heading), &(m_table->heading_row), m_logger, m_b_ok, m_parseconfig);
		std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
		visitor.CheckDefCounts();
	}
	else {
		m_logger->LogInformation("Table-Parser: ignoring thead-node \"" + xmlnode.name + "\".");
	}
}

//----------------------------------------------------------------------------------------------

void table_visitor::operator()(mini_xml &xmlnode)
{
	if(xmlnode.name == "thead") {
		m_def_counts->thead++;
		thead_visitor visitor(m_table, m_logger, m_b_ok, m_parseconfig);
		std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
		visitor.CheckDefCounts();
	}
	else if(xmlnode.name == "tbody") {
		m_def_counts->tbody++;
		tbody_visitor visitor(m_table, m_logger, m_b_ok, m_parseconfig);
		std::for_each(xmlnode.children.begin(), xmlnode.children.end(), boost::apply_visitor(visitor));
		visitor.CheckDefCounts();
	}
	else {
		m_logger->LogInformation("Table-Parser: ignoring thead-node \"" + xmlnode.name + "\".");
	}
}

//----------------------------------------------------------------------------------------------

void NormalWarning(unsigned int count, std::string name, logger_base_ptr c_logger) 
{
	if(count == 0) {
		c_logger->LogWarning("Table-Parser: no " + name + " defined. Default value is used.");
	} 
	else if(count > 1) {
		c_logger->LogWarning("Table-Parser: " + name + " defined multiply. The last defined value is used.");
	}
}
void TooManyWarning(unsigned int count, std::string name, bool weiblich, logger_base_ptr c_logger) 
{
	if(count > 1) {
		c_logger->LogWarning("Table-Parser: " + name + " defined multiply. The last defined value is used.");
	}
}
void TooManyInformation(unsigned int count, std::string name, bool weiblich, logger_base_ptr c_logger) 
{
	if(count > 1) {
		c_logger->LogInformation("Table-Parser: " + name + " defined multiply. The last defined value is used.");
	}
}
void NormalInformation(unsigned int count, std::string name, logger_base_ptr c_logger)
{
	if(count == 0) {
		c_logger->LogInformation("Table-Parser: no " + name + " defined. Default value is used.");
	} 
	else if(count > 1) {
		c_logger->LogWarning("Table-Parser: " + name + " defined multiply. The last defined value is used.");
	}
}
void NormalError(unsigned int count, std::string name, logger_base_ptr c_logger, bool *b_ok) 
{
	if(count == 0) {
		c_logger->LogError("Table-Parser: no " + name + " defined.");
		*b_ok = false;
	} 
	else if(count > 1) {
		c_logger->LogWarning("Table-Parser: " + name + " defined multiply. The last defined value is used.");
	}
}
void TooManyError(unsigned int count, std::string name, logger_base_ptr c_logger, bool *b_ok) 
{
	if(count > 1) {
		c_logger->LogError("Table-Parser: " + name + " defined multiply..");
		*b_ok = false;
	}
}

//----------------------------------------------------------------------------------------------

int Parse(std::string::const_iterator begin, std::string::const_iterator end, logger_base_ptr c_logger, html_table &c_table, bool b_inner_html_allowed)
{
    typedef mini_xml_grammar<std::string::const_iterator> grammar1;
    std::vector<mini_xml> tree;
    std::string::const_iterator iter = begin;
    error_handler<std::string::const_iterator> e(iter, end, c_logger);
    grammar1 g1(&e); // Our grammar

    /*{
        using boost::spirit::qi::lit;
        using boost::spirit::qi::lexeme;
        using boost::spirit::qi::on_error;
        using boost::spirit::qi::fail;
        using spirit_encoding::char_;
        using spirit_encoding::digit;
        using spirit_encoding::string;
        using spirit_encoding::space; // Matches spaces, tabs, returns, and newlines
        using spirit_encoding::punct; // also '(', ')'
        using boost::spirit::int_;
        using boost::spirit::float_;
        namespace qlabels = boost::spirit::qi::labels;

        // "\xc3\xb7"
        std::string str;
        std::cout << boost::spirit::qi::phrase_parse(iter, end,
                lexeme[*( (char_ - space - char_("\xc3=[\\]^{}") - char_('!', '@')) // '_' must be allowed
                        [boost::phoenix::ref(str) += qlabels::_1]
                    | (char_('\xc3')[boost::phoenix::ref(str) += qlabels::_1]
                            >> (char_ - space - char_("\xb7=[\\]^{}") - char_('!', '@'))[boost::phoenix::ref(str) += qlabels::_1]) )],
                space) << std::endl;
        std::cout << '"' << str << '"' << " " << (iter == end) << std::endl;
        iter = ausdruck_str.begin();
    }*/
    bool r = boost::spirit::qi::phrase_parse(iter, end,
                 g1, spirit_encoding::space, tree);
    if (r && (iter == end)) {
		bool b_ok = true;
		parseconfig c_parseconfig;
		c_parseconfig.b_inner_html_allowed = b_inner_html_allowed;
		table_visitor::def_counts top_def_counts;
		table_visitor visitor(&c_table, c_logger, &b_ok, &top_def_counts, &c_parseconfig);
		// The config tag is allowed to be opened and closed multiple times.
		for(std::vector<mini_xml>::iterator it_oberxml = tree.begin(); it_oberxml != tree.end(); it_oberxml++) {
			if((*it_oberxml).name == "table") {
				std::for_each((*it_oberxml).children.begin(), (*it_oberxml).children.end(), boost::apply_visitor(visitor));
			}
			else {
				c_logger->LogInformation("Table-Parser: Ignoring node \"" + (*it_oberxml).name + "\".");
			}
		}
		top_def_counts.Check(c_logger, &b_ok);
		if(b_ok) {
			return 0;
		}
		else {
			return 2;
		}
    }
    else {
        std::string::const_iterator it_errzeile_end = iter;
        while((it_errzeile_end != end) && (*it_errzeile_end != '\n') && (*it_errzeile_end != '\r')) {
            it_errzeile_end++;
        }
        unsigned int line;
        std::string::const_iterator it_line_begin = e.GetLine(begin, iter, &line);
        unsigned int column = 0;
        for(std::string::const_iterator it_column = it_line_begin; it_column != iter; it_column++) {
            if(*it_column == '\t') {
				column += 4;
			}
			else {
				column++;
			}
        }

		std::string errzeile(it_line_begin, it_errzeile_end);
		str_replace_char(errzeile, '\t', "    ");
        c_logger->LogError(std::string("Error while parsing line ") + Int_To_String(line) + " near char " + Int_To_String(column) + ":");
        c_logger->LogError(errzeile);
        c_logger->LogError(std::string(column, ' ') + '^');

        return 1;
    }
}

} // namespace html_table_parser
} // namespace stocks_dl
