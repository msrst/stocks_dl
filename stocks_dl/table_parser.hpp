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

#ifndef LOGGING_CONFIGPARSER_HPP_INCLUDED
#define LOGGING_CONFIGPARSER_HPP_INCLUDED

#include <iostream>

#include "utils/mini_xml_decl.hpp"
#include "utils/logger.hpp"
#include "decl.hpp"

namespace stocks_dl {

struct html_table
{
	std::string heading_heading;
	std::vector<std::string> heading_row;
	std::vector<std::string> heading_column;
	std::vector<std::vector<std::string>> zeilen;
	
	bool Check(logger_base_ptr c_logger, bool adjust_spaltenheading = false); // when adjust_spaltenheading is set, the
					// first column is moved to be the heading column if this makes the table valid.
	// Check must be true to use this functions.
	void Print();
	void RemoveSidewhites();
};

namespace html_table_parser {
	
struct parseconfig
{
	bool b_inner_html_allowed;
};

void NormalWarning(unsigned int count, std::string name, logger_base_ptr c_logger);
void NormalInformation(unsigned int count, std::string, logger_base_ptr c_logger);
void NormalError(unsigned int count, std::string name, logger_base_ptr c_logger, bool *b_ok);
void TooManyError(unsigned int count, std::string name, logger_base_ptr c_logger, bool *b_ok);
void TooManyWarning(unsigned int count, std::string name, logger_base_ptr c_logger);
void TooManyInformation(unsigned int count, std::string name, logger_base_ptr c_logger);

class head_tr_visitor:
    public boost::static_visitor<>
{
private:
	struct def_counts {
		int heading_heading = 0;
		
		void Check(logger_base_ptr c_logger, bool *b_ok) {
		}
	};
	
	std::string *m_heading;
	std::vector<std::string> *m_neue_zeile;
	logger_base_ptr m_logger;
	bool *m_b_ok;
	def_counts m_def_counts;
	parseconfig *m_parseconfig;
    
public:
    head_tr_visitor(std::string *heading, std::vector<std::string> *neue_zeile, logger_base_ptr c_logger, bool *b_ok, parseconfig *c_parseconfig) {
		m_heading = heading;
		m_neue_zeile = neue_zeile;
		m_logger = c_logger;
		m_b_ok = b_ok;
		m_parseconfig = c_parseconfig;
	}
    
    void operator()(mini_xml &xmlnode);
    void operator()(std::string &str) {
		m_logger->LogError("Table-Parser: tr-node of the thead-element: only text, no tags.");
		*m_b_ok = false;
	}
	
	void CheckDefCounts() {
		m_def_counts.Check(m_logger, m_b_ok);
	}
};
class body_tr_visitor:
    public boost::static_visitor<>
{
private:
	struct def_counts {
		unsigned int th = 0;
		
		void Check(logger_base_ptr c_logger, bool *b_ok) {
			TooManyError(th, "tbody-line-heading", c_logger, b_ok);
		}
	};
	
	std::string *m_heading;
	std::vector<std::string> *m_neue_zeile;
	logger_base_ptr m_logger;
	bool *m_b_ok;
	def_counts m_def_counts;
	parseconfig *m_parseconfig;
    
public:
    body_tr_visitor(std::string *heading, std::vector<std::string> *neue_zeile, logger_base_ptr c_logger, bool *b_ok, parseconfig *c_parseconfig) {
		m_heading = heading;
		m_neue_zeile = neue_zeile;
		m_logger = c_logger;
		m_b_ok = b_ok;
		m_parseconfig = c_parseconfig;
	}
    
    void operator()(mini_xml &xmlnode);
    void operator()(std::string &str) {
		m_logger->LogError("Table-Parser: tr-node of the tbody element: only text, no tags.");
		*m_b_ok = false;
	}
	
	void CheckDefCounts() {
		m_def_counts.Check(m_logger, m_b_ok);
	}
};

class thead_visitor:
    public boost::static_visitor<>
{
private:
	struct def_counts {
		void Check(logger_base_ptr c_logger, bool *b_ok) {
		}
	};
	
	html_table *m_table;
	logger_base_ptr m_logger;
	bool *m_b_ok;
	def_counts m_def_counts;
	parseconfig *m_parseconfig;
    
public:
    thead_visitor(html_table *c_table, logger_base_ptr c_logger, bool *b_ok, parseconfig *c_parseconfig) {
		m_table = c_table;
		m_logger = c_logger;
		m_b_ok = b_ok;
		m_parseconfig = c_parseconfig;
	}
    
    void operator()(mini_xml &xmlnode);
    void operator()(std::string &str) {
		m_logger->LogError("Table-Parser: thead node: only text, no tags.");
		*m_b_ok = false;
	}
	
	void CheckDefCounts() {
		m_def_counts.Check(m_logger, m_b_ok);
	}
};
class tbody_visitor:
    public boost::static_visitor<>
{
private:
	struct def_counts {
		void Check(logger_base_ptr c_logger, bool *b_ok) {
		}
	};
	
	html_table *m_table;
	logger_base_ptr m_logger;
	bool *m_b_ok;
	def_counts m_def_counts;
	parseconfig *m_parseconfig;
    
public:
    tbody_visitor(html_table *c_table, logger_base_ptr c_logger, bool *b_ok, parseconfig *c_parseconfig) {
		m_table = c_table;
		m_logger = c_logger;
		m_b_ok = b_ok;
		m_parseconfig = c_parseconfig;
	}
    
    void operator()(mini_xml &xmlnode);
    void operator()(std::string &str) {
		m_logger->LogError("Table-Parser: tbody-node: only text, no tags.");
		*m_b_ok = false;
	}
	
	void CheckDefCounts() {
		m_def_counts.Check(m_logger, m_b_ok);
	}
};

class table_visitor:
    public boost::static_visitor<>
{
public:
	struct def_counts {
		unsigned int thead = 0, tbody = 0;
		
		void Check(logger_base_ptr c_logger, bool *b_ok) {
			NormalError(thead, "thead", c_logger, b_ok);
			NormalError(tbody, "tbody", c_logger, b_ok);
		}
	};

private:
	html_table *m_table;
	logger_base_ptr m_logger;
	bool *m_b_ok;
	def_counts *m_def_counts;
	parseconfig *m_parseconfig;
    
public:
    table_visitor(html_table *c_table, logger_base_ptr c_logger, bool *b_ok, def_counts *c_def_counts, parseconfig *c_parseconfig) {
		m_table = c_table;
		m_logger = c_logger;
		m_b_ok = b_ok;
		m_def_counts = c_def_counts;
		m_parseconfig = c_parseconfig;
	}
    
    void operator()(mini_xml &xmlnode);
    void operator()(std::string &str) {
		m_logger->LogError("Table-Parser: table-Node: only text, no tags.");
		*m_b_ok = false;
	}
};

// 0 at success.
// At failure, it is possible that Parse(.) has changed c_config.
int Parse(std::string::const_iterator begin, std::string::const_iterator end, logger_base_ptr c_logger, html_table &c_table, bool b_inner_html_allowed = false);

} // namespace html_table_parser
} // namespace stocks_dl

#endif // LOGGING_CONFIGPARSER_HPP_INCLUDED
