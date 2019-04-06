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

#ifndef STOCKS_DL_MINI_XML_HPP_INCLUDED
#define STOCKS_DL_MINI_XML_HPP_INCLUDED

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include "../utils/logger.hpp"
#include "mini_xml_decl.hpp"

// We need to tell fusion about our mini_xml struct
// to make it a first-class fusion citizen
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::mini_xml,
    (std::string, name),
    (std::vector<stocks_dl::mini_xml_nodeparam>, parameter),
    (std::vector<stocks_dl::mini_xml_node>, children)
)

BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::mini_xml_nodeparam,
    (std::string, name),
    (std::string, value)
)

namespace stocks_dl {

// see also error_handler.hpp and statement_def.hpp from boost_root/libs/spirit/example/qi/compiler_tutorial/conjure3
template <typename Iterator>
struct error_handler
{
private:
    Iterator m_it_begin;
    Iterator m_it_end;
    logger_base_ptr m_logger;

public:
    error_handler(Iterator it_begin, Iterator it_end, logger_base_ptr c_logger)
    {
        m_it_begin = it_begin;
        m_it_end = it_end;
        m_logger = c_logger;
    }

    void handle_error(boost::fusion::vector<Iterator&, Iterator const&, Iterator const&, boost::spirit::info> args)
    {
        Iterator err_pos = boost::fusion::at_c<2>(args);
        //Iterator it_begin = boost::fusion::at_c<0>(args);
        //Iterator it_end = boost::fusion::at_c<1>(args);
        Iterator it_errorline_end = err_pos;
        while((it_errorline_end != m_it_end) && (*it_errorline_end != '\n') && (*it_errorline_end != '\r')) {
            it_errorline_end++;
        }
        unsigned int line;
        std::stringstream sstream;
        sstream << boost::fusion::at_c<3>(args);
        
        std::string::const_iterator it_line_begin = GetLine(m_it_begin, err_pos, &line);
        unsigned int column = 0;
        for(std::string::const_iterator it_column = it_line_begin; it_column != err_pos; it_column++) {
			if(*it_column == '\t') {
				column += 4;
			}
			else {
				column++;
			}
        }
        
        std::string str_errorline(it_line_begin, it_errorline_end);
        str_replace_char(str_errorline, '\t', std::string("    "));
        std::string str_warnung = std::string("Parser-Error zu ") + sstream.str() + ": Zeile " + Int_To_String(line)
				+ ", Spalte " + Int_To_String(column) + ": ";
        m_logger->LogWarning(str_warnung + str_errorline);
		m_logger->LogWarning(std::string(str_warnung.length() + column, ' ') + '^');
    }

    // Returns the beginning of the line
    Iterator GetLine(Iterator it_begin, Iterator it_err_pos, unsigned int* line) const
    {
        *line = 1;
        Iterator i = it_begin;
        Iterator line_start = it_begin;
        while (i != it_err_pos)
        {
            bool eol = false;
            if ((i != it_err_pos) && (*i == '\r')) // CR
            {
                eol = true;
                line_start = ++i;
            }
            if ((i != it_err_pos) && (*i == '\n')) // LF
            {
                eol = true;
                line_start = ++i;
            }
            if (eol)
                ++(*line);
            else
                ++i;
        }
        return line_start;
    }
};

class xml_value_visitor:
    public boost::static_visitor<>
{
private:
    long int value_stat; // 0: nothing read yet, 1: read exactly one value node,
                // 2: read more than one value node or read at least one xml node
    std::string m_value;
    
public:
    xml_value_visitor();
    
    void operator()(mini_xml& xmlnode);
    void operator()(std::string& str);
    
    bool GetValue(std::string* value);
};

class xml_htmltext_visitor:
    public boost::static_visitor<>
{
private:
    std::string *m_value;
    
public:
    xml_htmltext_visitor(std::string *c_value) {
		m_value = c_value;
	}
    
    void operator()(mini_xml& xmlnode);
    void operator()(std::string& str) {
		m_value->append(str);
	}
};

namespace spirit_encoding = boost::spirit::iso8859_1;

template <typename Iterator>
struct mini_xml_grammar
  : boost::spirit::qi::grammar<Iterator, std::vector<mini_xml>(), spirit_encoding::space_type>
{
    // see also www.w3.org/TR/xml/
    mini_xml_grammar(error_handler<Iterator>* e)
			: mini_xml_grammar::base_type(top_xml, "top_xml")
	{
		using boost::spirit::qi::lit;
		using boost::spirit::qi::lexeme;
		using boost::spirit::qi::on_error;
		using boost::spirit::qi::fail;
		using spirit_encoding::char_;
		using spirit_encoding::string;
		using spirit_encoding::space;
		namespace qlabels =  boost::spirit::qi::labels;

		using boost::phoenix::construct;
		using boost::phoenix::val;
		using boost::phoenix::at_c;
		using boost::phoenix::push_back;

		xml_type_tag = lit("<?xml")
				> *(char_ - '?')
				> "?>";
		doctype_tag = lit("<!DOCTYPE")
				> lexeme[ *( (char_ - ']') | (']' >> (char_ - '>')) ) ]
				> "]>";
		comment = lit("<!--") >> lexeme[*( (char_ - '-') | ('-' >> (char_ - '-')) | ("--" >> (char_ - '>')) )] >> "-->"; 

		text %= lexeme[+(lit("\\<")[qlabels::_val += '<'] | (char_ - '<'))];
		node %= xml | text;

		start_tag_begin %=
				'<'
			>>  !lit('/')
			>>  lexeme[+(char_ - (space | char_("<>/\"")))]
			//>   lexeme[+(char_ - ('>' | space)) [qlabels::_val += qlabels::_1]]
			//>   *(char_ - '>')
			//>   '>'
			;
		
		parameter = lexeme[+(char_ - char_("=/>")) [at_c<0>(qlabels::_val) += qlabels::_1]]
							  > '='
							  > parameterwert [at_c<1>(qlabels::_val) = qlabels::_1];
		start_tag_parameter %= *(parameter);

		end_tag =
				"</"
			>   string(qlabels::_r1)
			>   '>'
			;

		xml =
				start_tag_begin[at_c<0>(qlabels::_val) = qlabels::_1]
			>   start_tag_parameter[at_c<1>(qlabels::_val) = qlabels::_1]
			>   (( '>'
					>   *(comment | node[push_back(at_c<2>(qlabels::_val), qlabels::_1)])
					>   end_tag(at_c<0>(qlabels::_val)) )
			   | "/>");
			;
		top_xml = -xml_type_tag
				>> -doctype_tag
				>> *(comment
						| xml[ push_back(qlabels::_val, qlabels::_1) ]);
		
        parameterwert = lexeme['"' > *( (char_ - char_('"'))[qlabels::_val += qlabels::_1]
                | (lit('\\')
                        > char_("'\\\""))[qlabels::_val += qlabels::_1]
                | (lit('\\')
                        > (char_ - char_("'\\\"")))[qlabels::_val += ',', qlabels::_val += qlabels::_1] ) > '"']
                       | lexeme['\'' > *( (char_ - char_('\''))[qlabels::_val += qlabels::_1]
                | (lit('\\')
                        > char_("'\\'"))[qlabels::_val += qlabels::_1]
                | (lit('\\')
                        > (char_ - char_("'\\'")))[qlabels::_val += ',', qlabels::_val += qlabels::_1] ) > '\''];

		top_xml.name("top_xml");
		xml.name("xml");
		node.name("node");
		text.name("text");
		start_tag_begin.name("start_tag");
		end_tag.name("end_tag");
		doctype_tag.name("doctype");
		xml_type_tag.name("xml_type_tag");
		parameter.name("parameter");
		start_tag_parameter.name("start_tag_parameter");
		comment.name("comment");

		on_error<fail>(xml,
			//std::cout
			//    << val("Error! Expecting ")
			//    << _4                               // what failed?
			//    << val(" here: \"")
			//    << construct<std::string>(_3, _2)   // iterators to error-pos, end
			//    << val("\"")
			//    << std::endl
			//boost::bind(mini_xml_grammar<Iterator>::handler, std::placeholders::_1, std::placeholders::_2)
			//boost::phoenix::function<error_handler<Iterator> >(e)(qlabels::_1, qlabels::_2, qlabels::_3, qlabels::_4)
			boost::bind(&error_handler<Iterator>::handle_error, e, _1)
			);
		//on_success(xml,
		//    boost::bind(&error_handler<Iterator>::check, &e, _1)
		//    );
		
	}

    boost::spirit::qi::rule<Iterator, mini_xml(), spirit_encoding::space_type> xml;
    boost::spirit::qi::rule<Iterator, std::vector<mini_xml>(), spirit_encoding::space_type> top_xml;
    boost::spirit::qi::rule<Iterator, mini_xml_node(), spirit_encoding::space_type> node;
    boost::spirit::qi::rule<Iterator, std::string(), spirit_encoding::space_type> text;
    boost::spirit::qi::rule<Iterator, std::string(), spirit_encoding::space_type> start_tag_begin;
    boost::spirit::qi::rule<Iterator, std::vector<mini_xml_nodeparam>(), spirit_encoding::space_type> start_tag_parameter;
    boost::spirit::qi::rule<Iterator, mini_xml_nodeparam(), spirit_encoding::space_type> parameter;
    boost::spirit::qi::rule<Iterator, void(std::string), spirit_encoding::space_type> end_tag;
    boost::spirit::qi::rule<Iterator, void(), spirit_encoding::space_type> doctype_tag;
    boost::spirit::qi::rule<Iterator, void(), spirit_encoding::space_type> xml_type_tag;
    boost::spirit::qi::rule<Iterator, void(), spirit_encoding::space_type> comment;
    boost::spirit::qi::rule<Iterator, std::string(), spirit_encoding::space_type> parameterwert;
};

} // namespace stocks_dl

#endif // STOCKS_DL_MINI_XML_HPP_INCLUDED
