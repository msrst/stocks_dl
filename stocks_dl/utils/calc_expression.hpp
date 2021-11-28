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
#ifndef AUSDRUCK_HPP_INCLUDED
#define AUSDRUCK_HPP_INCLUDED

#include <vector>
#include <string>
#include <sstream>
#include <cmath>

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
//#include <boost/variant/recursive_variant.hpp>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/bind/bind.hpp>

#include "utils.hpp"
#include "logger.hpp"
#include "../decl.hpp"

inline double allnum_pow(const double& a, const double& b) {
    return std::pow(a, b);
}

namespace stocks_dl {
namespace expression_parser {

// siehe auch error_handler.hpp und statement_def.hpp aus /home/matthias/cpp_ubuntu/boost_1_61_0ub/libs/spirit/example/qi/compiler_tutorial/conjure3
template <typename Iterator>
struct error_handler
{
private:
    Iterator m_it_begin;
    Iterator m_it_end;
    logger_base* m_logger;

public:
    error_handler(Iterator it_begin, Iterator it_end, logger_base* c_logger)
    {
        m_it_begin = it_begin;
        m_it_end = it_end;
        m_logger = c_logger;
    }

    /*template <typename Telementwhat>
    void operator()(Iterator it_begin, Iterator it_end, Iterator err_pos, Telementwhat elementwhat) const
    {
        Iterator it_errzeile_end = err_pos;
        while((it_errzeile_end != m_it_end) && (*it_errzeile_end != '\n') && (*it_errzeile_end != '\r')) {
            it_errzeile_end++;
        }
        unsigned int line;
        std::string zeilenstring(GetLine(m_it_begin, err_pos, &line), it_errzeile_end);
        std::cout << "Parser-Error \"" << elementwhat << "\": Linie " << line << ": " << zeilenstring << std::endl;
    }

    void check(boost::fusion::vector<Iterator&, Iterator const&, Iterator const&> args)
    {}*/
    void handle_error(boost::fusion::vector<Iterator&, Iterator const&, Iterator const&, boost::spirit::info> args)
    {
        Iterator err_pos = boost::fusion::at_c<2>(args);
        //Iterator it_begin = boost::fusion::at_c<0>(args);
        //Iterator it_end = boost::fusion::at_c<1>(args);
        Iterator it_errzeile_end = err_pos;
        while((it_errzeile_end != m_it_end) && (*it_errzeile_end != '\n') && (*it_errzeile_end != '\r')) {
            it_errzeile_end++;
        }
        unsigned int line;
        std::stringstream sstream;
        sstream << boost::fusion::at_c<3>(args);
        std::string zeilenstring(GetLine(m_it_begin, err_pos, &line), it_errzeile_end);
        m_logger->LogWarning(std::string("Parser-Error zu ") + sstream.str() + ": Zeile " + Int_To_String(line) + ": " + zeilenstring);
    }

    // Gibt den Anfang der Linie zurück
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

struct float_unconverted
{
    std::string vor_komma;
    std::string nach_komma;
};

struct expression_braces;

struct ausdruck_pot;
typedef
    boost::variant<
        expression_braces,
        float_unconverted
    >
teilausdruck_pot_in;
struct ausdruck_pot
{
    std::vector<teilausdruck_pot_in> v;
};

typedef
    boost::variant<
        expression_braces,
        ausdruck_pot,
        float_unconverted
    >
teilausdruck_mul_in;
struct teilausdruck_mul_bi;
struct ausdruck_mul
{
    std::vector<teilausdruck_mul_bi> v;
};

typedef
    boost::variant<
        expression_braces,
        ausdruck_mul,
        ausdruck_pot,
        float_unconverted
    >
teilausdruck_add_in;
struct teilausdruck_add_bi;
struct ausdruck_add
{
    std::vector<teilausdruck_add_bi> v;
};

typedef
    boost::variant<
        expression_braces,
        ausdruck_add,
        ausdruck_mul,
        ausdruck_pot,
        float_unconverted
    >
teilausdruck_klammer_in;
struct expression_braces
{
    std::vector<teilausdruck_klammer_in> argumente;
};

typedef
    boost::variant<
        expression_braces,
        ausdruck_add,
        ausdruck_mul,
        ausdruck_pot,
        float_unconverted
    >
teilausdruck_top;

struct teilausdruck_add_bi
{
    bool b_plus;
    teilausdruck_add_in v;
};
struct teilausdruck_mul_bi
{
    bool b_mal;
    teilausdruck_mul_in v;
};

} // namespace expression_parser
} // namespace stocks_dl

// We need to tell fusion about our struct to make it a first-class fusion citizen
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::float_unconverted,
    (std::string, vor_komma),
    (std::string, nach_komma)
)
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::ausdruck_pot,
    (std::vector<stocks_dl::expression_parser::teilausdruck_pot_in>, v)
)
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::teilausdruck_mul_bi,
    (bool, b_mal)
    (stocks_dl::expression_parser::teilausdruck_mul_in, v)
)
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::ausdruck_mul,
    (std::vector<stocks_dl::expression_parser::teilausdruck_mul_bi>, v)
)
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::teilausdruck_add_bi,
    (bool, b_plus)
    (stocks_dl::expression_parser::teilausdruck_add_in, v)
)
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::ausdruck_add,
    (std::vector<stocks_dl::expression_parser::teilausdruck_add_bi>, v)
)
BOOST_FUSION_ADAPT_STRUCT(
    stocks_dl::expression_parser::expression_braces,
    (std::vector<stocks_dl::expression_parser::teilausdruck_klammer_in>, argumente)
)

namespace stocks_dl {
namespace expression_parser {

namespace spirit_encoding = boost::spirit::iso8859_1;

template <typename Iterator>
struct grammar1
  : boost::spirit::qi::grammar<Iterator, teilausdruck_top(), spirit_encoding::space_type>
{
    grammar1(error_handler<Iterator>* e)
        : grammar1::base_type(top, "top")
    {
        using boost::spirit::qi::lit;
        using boost::spirit::qi::lexeme;
        using boost::spirit::qi::on_error;
        using boost::spirit::qi::fail;
        using spirit_encoding::char_;
        using spirit_encoding::print;
        using spirit_encoding::digit;
        using spirit_encoding::string;
        using spirit_encoding::space; // Matches spaces, tabs, returns, and newlines
        using spirit_encoding::punct; // auch '(', ')'
        using boost::spirit::int_;
        using boost::spirit::float_;
        namespace qlabels =  boost::spirit::qi::labels;

        using boost::phoenix::construct;
        using boost::phoenix::val;
        using boost::phoenix::at_c;
        using boost::phoenix::push_back;

        mfloat %= lexeme[ (+digit) > -(lit('.') > (*digit)) ];
        add = (+(add_summand[push_back(at_c<0>(qlabels::_val), qlabels::_1)]))
                | (add_frontsummandnichts[push_back(at_c<0>(qlabels::_val), qlabels::_1)]
                        >> +(add_summand[push_back(at_c<0>(qlabels::_val), qlabels::_1)]));
        add_summand = (char_('+')[at_c<0>(qlabels::_val) = true]
                        >> teilterm_add[at_c<1>(qlabels::_val) = qlabels::_1])
                | (char_('-')[at_c<0>(qlabels::_val) = false]
                        >> teilterm_add[at_c<1>(qlabels::_val) = qlabels::_1]);
        add_frontsummandnichts = teilterm_add[at_c<0>(qlabels::_val) = true, at_c<1>(qlabels::_val) = qlabels::_1];
        mul = mul_frontfaktor[push_back(at_c<0>(qlabels::_val), qlabels::_1)]
                >> +(mul_faktor[push_back(at_c<0>(qlabels::_val), qlabels::_1)]);
        mul_faktor = (char_('*')[at_c<0>(qlabels::_val) = true] >>
                        teilterm_mul[at_c<1>(qlabels::_val) = qlabels::_1])
                | (char_('/')[at_c<0>(qlabels::_val) = false]
                        >> teilterm_mul[at_c<1>(qlabels::_val) = qlabels::_1]);
        mul_frontfaktor = teilterm_mul[at_c<0>(qlabels::_val) = true, at_c<1>(qlabels::_val) = qlabels::_1];
        pot = teilterm_pot[push_back(at_c<0>(qlabels::_val), qlabels::_1)]
                >> +('^'
                        >> teilterm_pot[push_back(at_c<0>(qlabels::_val), qlabels::_1)]);
        klammer %= '('
                >> (-(klammerinhalt % ','))
                >> ')';
        // bei folgender Konstellation: (((((1))))), nicht jedoch bei (1+(1+(1+(1+(1+1))))) oder (((((1+1)+1)+1)+1)+1), lahmt der Parser extrem (4s parsezeit).
        // Das kann man Ändern, wenn man klammer immer an den Anfang der Oder-Kette stellt, aber dann parst er z. B. (1)*3 nicht mehr korrekt.
        top %= add | mul | pot | klammer | mfloat;
        klammerinhalt %= add | mul | pot | klammer | mfloat;
        teilterm_add %= mul | pot | klammer | mfloat;
        teilterm_mul %= pot | klammer | mfloat;
        teilterm_pot %= klammer | mfloat;
        // Es muss trotz lexeme[..] char_ - space heißen, weil char_ auch spaces einschließt.
        // Ich kann nicht direkt *(graph - digit - punct) schreiben, weil der iso-kram, der ermöglicht, dass unicode-teilzeichen akzeptiert werden, nicht druckbare zeichen enthält.
        name = lexeme[(char_ - space - char_("=[\\]^{}") - char_('!', '@'))[qlabels::_val += qlabels::_1] > (*( (char_ - space - char_("=[\\]^{}:;<=>?@") - char_('!', '/')) // '_' muss erlaubt sein. char_('!', '@') nimmt alle Zeichen von '!' bis '@'
                            [qlabels::_val += qlabels::_1]))];

        add.name("add");
        add_summand.name("add_summand");
        add_frontsummandnichts.name("add_frontsummandnichts");
        mul.name("mul");
        mul_faktor.name("mul_faktor");
        mul_frontfaktor.name("mul_frontfaktor");
        pot.name("pot");
        klammer.name("klammer");
        top.name("top");
        klammerinhalt.name("klammerinhalt");
        teilterm_add.name("teilterm_add");
        teilterm_mul.name("teilterm_mul");
        teilterm_pot.name("teilterm_pot");
        name.name("name");
        on_error<fail>(klammerinhalt,
            //std::cout
            //    << val("Error! Expecting ")
            //    << _4                               // what failed?
            //    << val(" here: \"")
            //    << construct<std::string>(_3, _2)   // iterators to error-pos, end
            //    << val("\"")
            //    << std::endl
            //boost::bind(mini_xml_grammar<Iterator>::handler, std::placeholders::_1, std::placeholders::_2)
            //boost::phoenix::function<error_handler<Iterator> >(e)(qlabels::_1, qlabels::_2, qlabels::_3, qlabels::_4)
            // boost::bind(&error_handler<Iterator>::handle_error, e, _1)  # its is depracted to use _1
            boost::bind(&error_handler<Iterator>::handle_error, e, boost::placeholders::_1)
            );
        //on_success(xml,
        //    boost::bind(&error_handler<Iterator>::check, &e, _1)
        //    );

    }

    boost::spirit::qi::rule<Iterator, ausdruck_add(), spirit_encoding::space_type> add;
    boost::spirit::qi::rule<Iterator, teilausdruck_add_bi(), spirit_encoding::space_type> add_frontsummandnichts;
    boost::spirit::qi::rule<Iterator, teilausdruck_add_bi(), spirit_encoding::space_type> add_summand;
    boost::spirit::qi::rule<Iterator, ausdruck_mul(), spirit_encoding::space_type> mul;
    boost::spirit::qi::rule<Iterator, teilausdruck_mul_bi(), spirit_encoding::space_type> mul_frontfaktor;
    boost::spirit::qi::rule<Iterator, teilausdruck_mul_bi(), spirit_encoding::space_type> mul_faktor;
    boost::spirit::qi::rule<Iterator, ausdruck_pot(), spirit_encoding::space_type> pot;
    boost::spirit::qi::rule<Iterator, expression_braces(), spirit_encoding::space_type> klammer;
    boost::spirit::qi::rule<Iterator, teilausdruck_top(), spirit_encoding::space_type> top;
    boost::spirit::qi::rule<Iterator, float_unconverted(), spirit_encoding::space_type> mfloat;
    boost::spirit::qi::rule<Iterator, teilausdruck_klammer_in(), spirit_encoding::space_type> klammerinhalt;
    boost::spirit::qi::rule<Iterator, teilausdruck_add_in(), spirit_encoding::space_type> teilterm_add;
    boost::spirit::qi::rule<Iterator, teilausdruck_mul_in(), spirit_encoding::space_type> teilterm_mul;
    boost::spirit::qi::rule<Iterator, teilausdruck_pot_in(), spirit_encoding::space_type> teilterm_pot;
    boost::spirit::qi::rule<Iterator, std::string(), spirit_encoding::space_type> name;
};

} // namespace expression_parser

struct mtypevoid_variant
{
};

namespace expression_parser {

template <class Tzahl> class term_add;
template <class Tzahl> class term_mul;
template <class Tzahl> class term_pot;
template <class Tzahl> class term_float;
template <class Tzahl> class term_braces;

template<class Tzahl>
class term
{
private:
    bool b_parser_err;

public:
    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    term<Tzahl>() {
        b_parser_err = false;
    }
    virtual ~term() {}

    void PrintForError(std::stringstream& sstream, std::vector<unsigned int>& fehler_stellen) {
        if(b_parser_err) {
            fehler_stellen.push_back(sstream.str().length());
        }
        DoPrintForError(sstream, fehler_stellen);
    }

    virtual expression_value Evaluate(bool *fehler) = 0;
    virtual bool GetSimpleVarReference(std::string &name) = 0;

public:
    void SetParserErr() {
        b_parser_err = true;
    }

    virtual void DoPrintForError(std::stringstream& ostream, std::vector<unsigned int> &fehler_stellen) = 0; // Die Fehler-Stellen werden von dieser Klasse gemanaged.
};
template<class Tzahl>
class term_visitor:
    public boost::static_visitor<>
{
private:
    term<Tzahl> **m_neuer_term;
    logger_base *m_logger;
    bool *m_b_ok;

public:
    term_visitor(term<Tzahl> **neuer_term, logger_base* c_logger, bool *b_ok) { // m_neuer_term wird bei jedem Aufruf von () ein neu allokierter Term zugeordnet (auch bei einem Parserfehler).
        m_neuer_term = neuer_term;
        m_logger = c_logger;
        m_b_ok = b_ok;
    }

    void operator()(const expression_parser::ausdruck_add& c_ausdruck) {
        term_add<Tzahl>* neuer_term = new term_add<Tzahl>(m_logger);
        if(!neuer_term->SetFromParsed(&c_ausdruck)) {
            *m_b_ok = false;
        }
        *m_neuer_term = neuer_term;
    }
    void operator()(const expression_parser::ausdruck_mul& c_ausdruck) {
        term_mul<Tzahl>* neuer_term = new term_mul<Tzahl>(m_logger);
        if(!neuer_term->SetFromParsed(&c_ausdruck)) {
            *m_b_ok = false;
        }
        *m_neuer_term = neuer_term;
    }
    void operator()(const expression_parser::ausdruck_pot& c_ausdruck) {
        term_pot<Tzahl>* neuer_term = new term_pot<Tzahl>(m_logger);
        if(!neuer_term->SetFromParsed(&c_ausdruck)) {
            *m_b_ok = false;
        }
        *m_neuer_term = neuer_term;
    }
    void operator()(const expression_parser::expression_braces& c_ausdruck) {
		term_braces<Tzahl>* neuer_term = new term_braces<Tzahl>(m_logger);
		if(!neuer_term->SetFromParsed(&c_ausdruck)) {
			*m_b_ok = false;
		}
		*m_neuer_term = neuer_term;
    }
    void operator()(const expression_parser::float_unconverted& c_ausdruck) {
        term_float<Tzahl>* neuer_term = new term_float<Tzahl>(m_logger);
        if(!neuer_term->SetFromParsed(&c_ausdruck)) {
            *m_b_ok = false;
        }
        *m_neuer_term = neuer_term;
    }
};

template<class Tzahl>
bool wert_as_zahl(Tzahl &wert, Tzahl &zahl) {
	zahl = wert;
	return true;
}

template<class Tzahl>
class summand
{
public:
    bool b_plus;
    term<Tzahl>* c_term;
};
template<class Tzahl>
class term_add:
    public term<Tzahl>
{
private:
    logger_base *m_logger;
    std::vector<summand<Tzahl>> summanden;

public:
    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    term_add(logger_base* c_logger) {
        m_logger = c_logger;
    }

    virtual ~term_add() {
        for(typename std::vector<summand<Tzahl>>::iterator it_teil = summanden.begin(); it_teil != summanden.end(); it_teil++) {
            delete (*it_teil).c_term;
        }
    }

    bool SetFromParsed(const expression_parser::ausdruck_add* parsed) {
        bool ret = true;
        summand<Tzahl> neuer_teil;
        term_visitor<Tzahl> c_visitor(&(neuer_teil.c_term), m_logger, &ret);
        for(std::vector<expression_parser::teilausdruck_add_bi>::const_iterator it_teil = parsed->v.begin();
                it_teil != parsed->v.end(); it_teil++) {
            neuer_teil.b_plus = (*it_teil).b_plus;
            boost::apply_visitor(c_visitor, (*it_teil).v);
            summanden.push_back(neuer_teil);
        }
        return ret;
    }

    class evaluator_visitor:
        public boost::static_visitor<>
    {
    private:
        term_add *m_parent;
        bool *m_fehler;

    public:
        bool b_first;
        Tzahl erg;
        bool b_plus;

        evaluator_visitor(term_add *parent, bool *fehler) {
            m_parent = parent;
            m_fehler = fehler;
            b_first = true;
        }

        void operator()(Tzahl &c_zahl) {
            if(b_first) {
                if(b_plus) {
                    erg = c_zahl;
                }
                else {
                    erg = -c_zahl;
                }
                b_first = false;
            }
            else {
                if(b_plus) {
					erg += c_zahl;
                }
                else {
					erg -= c_zahl;
                }
            }
        }
        void operator()(const mtypevoid_variant& c_void) {
            m_parent->m_logger->LogError("leerer Typ sollte addiert werden.");
            m_parent->SetParserErr();
            *m_fehler = true;
        }
    };

    virtual expression_value Evaluate(bool *fehler) {
        if(summanden.empty() == false) {
            evaluator_visitor c_visitor(this, fehler);
            for(typename std::vector<summand<Tzahl>>::iterator it_teil = summanden.begin(); it_teil != summanden.end(); it_teil++) {
                c_visitor.b_plus = (*it_teil).b_plus;
                expression_value w = (*it_teil).c_term->Evaluate(fehler);
                boost::apply_visitor(c_visitor, w);
            }
            if(c_visitor.b_first) {
                return static_cast<Tzahl>(1);
            }
            else {
                return c_visitor.erg;
            }
        }
        else {
            return mtypevoid_variant();
        }
    }
    virtual bool GetSimpleVarReference(std::string &name) {
        if(summanden.size() == 1) {
            if(summanden[0].b_plus == true) {
                return summanden[0].c_term->GetSimpleVarReference(name);
            }
        }
        return false;
    }

private:
    virtual void DoPrintForError(std::stringstream& ostream, std::vector<unsigned int> &fehler_stellen) {
        ostream << "add" << summanden.size() << "<";
        if(!(summanden.empty())) {
            typename std::vector<summand<Tzahl>>::iterator it_teil = summanden.begin();
            if(it_teil != summanden.end()) {
                if((*it_teil).b_plus) {
                    (*it_teil).c_term->PrintForError(ostream, fehler_stellen);
                }
                else {
                    ostream << '-';
                    (*it_teil).c_term->PrintForError(ostream, fehler_stellen);
                }
                for(it_teil++; it_teil != summanden.end(); it_teil++) {
                    ostream << ((*it_teil).b_plus ? '+' : '-');
                    (*it_teil).c_term->PrintForError(ostream, fehler_stellen);
                }
            }
        }
        ostream << ">";
    }
};

template<class Tzahl>
class factor
{
public:
    bool b_mal;
    term<Tzahl>* c_term;
};
template<class Tzahl>
class term_mul:
    public term<Tzahl>
{
private:
    logger_base *m_logger;
    std::vector<factor<Tzahl>> faktoren;

public:
    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    term_mul(logger_base* c_logger) {
        m_logger = c_logger;
    }
    virtual ~term_mul() {
        for(typename std::vector<factor<Tzahl>>::iterator it_teil = faktoren.begin(); it_teil != faktoren.end(); it_teil++) {
            delete (*it_teil).c_term;
        }
    }

    bool SetFromParsed(const expression_parser::ausdruck_mul* parsed) {
        bool ret = true;
        factor<Tzahl> neuer_teil;
        term_visitor<Tzahl> c_visitor(&(neuer_teil.c_term), m_logger, &ret);
        for(typename std::vector<expression_parser::teilausdruck_mul_bi>::const_iterator it_teil = parsed->v.begin();
                it_teil != parsed->v.end(); it_teil++) {
            neuer_teil.b_mal = (*it_teil).b_mal;
            boost::apply_visitor(c_visitor, (*it_teil).v);
            faktoren.push_back(neuer_teil);
        }
        return ret;
    }

    class evaluator_visitor:
        public boost::static_visitor<>
    {
    private:
        term_mul<Tzahl> *m_parent;
        bool *m_fehler;

    public:
        bool b_first;
        factor<Tzahl>* c_faktor;
        Tzahl erg;

        evaluator_visitor(term_mul<Tzahl> *parent, bool *fehler) {
            m_parent = parent;
            m_fehler = fehler;
            b_first = true;
        }

        void operator()(Tzahl &c_zahl) {
            if(b_first) {
                erg = c_zahl;
                b_first = false;
            }
            else {
                if(c_faktor->b_mal) {
                    erg *= c_zahl;
                }
                else {
					if(c_zahl == 0) {
						m_parent->m_logger->LogError("Es sollte durch 0 geteilt werden.");
						m_parent->SetParserErr();
						*m_fehler = true;
					}
					else {
						erg /= c_zahl;
					}
                }
            }
        }
        void operator()(const mtypevoid_variant& c_void) {
            m_parent->m_logger->LogError("leerer Typ sollte multipliziert werden.");
            c_faktor->c_term->SetParserErr();
            *m_fehler = true;
        }
    };

    virtual expression_value Evaluate(bool *fehler) {
        if(faktoren.empty() == false) {
            evaluator_visitor c_visitor(this, fehler);
            for(typename std::vector<factor<Tzahl>>::iterator it_teil = faktoren.begin(); it_teil != faktoren.end(); it_teil++) {
                c_visitor.c_faktor = &(*it_teil);
                expression_value w = (*it_teil).c_term->Evaluate(fehler);
                boost::apply_visitor(c_visitor, w);
            }
            if(c_visitor.b_first) {
                return static_cast<Tzahl>(1);
            }
            else {
                return c_visitor.erg;
            }
        }
        else {
            return mtypevoid_variant();
        }
    }
    virtual bool GetSimpleVarReference(std::string &name) {
        return false;
    }

private:
    virtual void DoPrintForError(std::stringstream& ostream, std::vector<unsigned int> &fehler_stellen)
    {
        ostream << "mul" << faktoren.size() << "<";
        if(!(faktoren.empty())) {
            typename std::vector<factor<Tzahl>>::iterator it_teil = faktoren.begin();
            if(it_teil != faktoren.end()) {
                (*it_teil).c_term->PrintForError(ostream, fehler_stellen);
                for(it_teil++; it_teil != faktoren.end(); it_teil++) {
                    ostream << ((*it_teil).b_mal ? '*' : '/');
                    (*it_teil).c_term->PrintForError(ostream, fehler_stellen);
                }
            }
        }
        ostream << ">";
    }
};

template<class Tzahl>
class term_pot:
    public term<Tzahl>
{
private:
    logger_base *m_logger;
    std::vector<term<Tzahl>*> teile;

public:
    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    term_pot(logger_base* c_logger) {
        m_logger = c_logger;
    }
    virtual ~term_pot() {
        for(typename std::vector<term<Tzahl>*>::iterator it_teil = teile.begin(); it_teil != teile.end(); it_teil++) {
            delete (*it_teil);
        }
    }

    bool SetFromParsed(const expression_parser::ausdruck_pot* parsed) {
        bool ret = true;
        term<Tzahl>* neuer_teil;
        term_visitor<Tzahl> c_visitor(&neuer_teil, m_logger, &ret);
        for(typename std::vector<expression_parser::teilausdruck_pot_in>::const_iterator it_teil = parsed->v.begin();
                it_teil != parsed->v.end(); it_teil++) {
            boost::apply_visitor(c_visitor, *it_teil);
            teile.push_back(neuer_teil);
        }
        return ret;
    }

    class evaluator_vars
    {
    public:
        term_pot<Tzahl> *parent;
        term<Tzahl>* c_teil;
        bool *m_fehler;
        Tzahl erg;
    };
    class evaluator_visitor_first:
        public boost::static_visitor<>
    {
    private:
        evaluator_vars *m_evars;

    public:
        evaluator_visitor_first(evaluator_vars *evars) {
            m_evars = evars;
        }

        void operator()(Tzahl &c_zahl) {
			m_evars->erg = c_zahl;
        }
        void operator()(const mtypevoid_variant& c_void) {
            m_evars->parent->m_logger->LogError("leerer Typ sollte potenziert werden.");
            m_evars->c_teil->SetParserErr();
            *(m_evars->m_fehler) = true;
            m_evars->erg = static_cast<Tzahl>(0);
        }
    };
    // Muss von rechts nach links aufgerufen werden
    class evaluator_visitor_zahl:
        public boost::static_visitor<>
    {
    private:
        evaluator_vars *m_evars;
        bool b_first;

    public:
        evaluator_visitor_zahl(evaluator_vars *evars) {
            m_evars = evars;
            b_first = true;
        }
        Tzahl erg;

        void operator()(Tzahl &c_zahl) {
			if(b_first) {
				erg = c_zahl;
				b_first = false;
			}
			else {
				if((erg == 0) && (c_zahl == 0)) {
					m_evars->parent->m_logger->LogError("0 hoch 0 ist nicht definiert.");
					m_evars->c_teil->SetParserErr();
					erg = 1; // 0^0 geht gegen 1 (damit weniger Folgefehler entstehen)
					*(m_evars->m_fehler) = true;
				}
				else {
					erg = allnum_pow(c_zahl, erg);
				}
			}
        }
        void operator()(const mtypevoid_variant& c_void) {
            m_evars->parent->m_logger->LogError("leerer Typ sollte potenziert werden.");
            m_evars->c_teil->SetParserErr();
            *(m_evars->m_fehler) = true;
        }
    };

    virtual expression_value Evaluate(bool *fehler) {
        if(teile.empty()) {
            return mtypevoid_variant();
        }
        else {
            evaluator_vars evars;
            evars.parent = this;
            evars.m_fehler = fehler;
            evaluator_visitor_first c_visitor(&evars);
            typename std::vector<term<Tzahl>*>::iterator it_basis = teile.begin();
            evars.c_teil = *it_basis;
            expression_value w = (*it_basis)->Evaluate(fehler);
            boost::apply_visitor(c_visitor, w);
            it_basis++;
            if(it_basis != teile.end()) {
                evaluator_visitor_zahl c_visitor_zahl(&evars);
                typename std::vector<term<Tzahl>*>::iterator it_exponent = teile.end();
                while(it_exponent != it_basis) {
                    it_exponent--;
                    evars.c_teil = *it_exponent;
                    w = (*it_exponent)->Evaluate(fehler);
                    boost::apply_visitor(c_visitor_zahl, w);
                }
				if((evars.erg == 0) && (c_visitor_zahl.erg == 0)) {
					m_logger->LogError("0 hoch 0 ist nicht definiert.");
					evars.c_teil->SetParserErr();
					*fehler = true;
					return static_cast<Tzahl>(1); // 0^0 geht gegen 1 (damit weniger Folgefehler entstehen)
				}
				else {
					return allnum_pow(evars.erg, c_visitor_zahl.erg);
				}
            }
            else {
                 return evars.erg;
            }
        }
    }
    virtual bool GetSimpleVarReference(std::string &name) {
        return false;
    }

private:
    virtual void DoPrintForError(std::stringstream& ostream, std::vector<unsigned int> &fehler_stellen)
    {
        ostream << "hoch" << teile.size() << "<";
        if(!(teile.empty())) {
            typename std::vector<term<Tzahl>*>::iterator it_teil = teile.begin();
            (*it_teil)->PrintForError(ostream, fehler_stellen);
            for(it_teil++; it_teil != teile.end(); it_teil++) {
                ostream << '^';
                (*it_teil)->PrintForError(ostream, fehler_stellen);
            }
        }
        ostream << ">";
    }
};

template<class Tzahl>
class term_float:
    public term<Tzahl>
{
private:
    logger_base *m_logger;
    expression_parser::float_unconverted m_unconverted; // For printing: on error, the number should look (nearly) like it was read before
    Tzahl val;

public:
    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    term_float(logger_base *c_logger) {
        m_logger = c_logger;
    }

    bool SetFromParsed(const expression_parser::float_unconverted* parsed) {
        m_unconverted = *parsed;
        string_to_int_converter convertobj;
        val = convertobj.ConvertToDouble(parsed->vor_komma + '.' + parsed->nach_komma);
        if(convertobj.MistakeHappened()) {
			m_logger->LogError("Could not convert number \"" + parsed->vor_komma + '.' + parsed->nach_komma + "\".");
			this->SetParserErr();
			return false;
		}
        else {
			return true;
		}
    }

    virtual expression_value Evaluate(bool *fehler) {
		return val;
    }
    virtual bool GetSimpleVarReference(std::string &name) {
        return false;
    }

private:
    virtual void DoPrintForError(std::stringstream& ostream, std::vector<unsigned int> &fehler_stellen) {
        ostream << m_unconverted.vor_komma;
        if(!m_unconverted.nach_komma.empty()) {
            ostream << '.' << m_unconverted.nach_komma;
        }
    }
};

template<class Tzahl>
class term_braces:
    public term<Tzahl>
{
private:
    logger_base *m_logger;
    std::vector<term<Tzahl>*> rohterme;

public:
    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    term_braces(logger_base* c_logger) {
        m_logger = c_logger;
    }
    virtual ~term_braces() {
        for(typename std::vector<term<Tzahl>*>::iterator it_teil = rohterme.begin(); it_teil != rohterme.end(); it_teil++) {
            delete (*it_teil);
        }
    }

    bool SetFromParsed(const expression_parser::expression_braces* parsed) {
        bool ret = true;
        term<Tzahl>* neuer_teil;
        term_visitor<Tzahl> c_visitor(&neuer_teil, m_logger, &ret);
        for(typename std::vector<expression_parser::teilausdruck_klammer_in>::const_iterator it_teil = parsed->argumente.begin();
                it_teil != parsed->argumente.end(); it_teil++) {
            boost::apply_visitor(c_visitor, *it_teil);
            rohterme.push_back(neuer_teil);
        }
        if(rohterme.size() > 1) {
            m_logger->LogError("Function without name or brace that contains a ','.");
            this->SetParserErr();
            ret = false;
        }
        else if(rohterme.size() == 0) {
            m_logger->LogError("Empty braces.");
            this->SetParserErr();
            ret = false;
        }
        return ret;
    }

    virtual expression_value Evaluate(bool *fehler) {
        if(rohterme.empty()) {
            return mtypevoid_variant();
        }
        else {
            return rohterme[0]->Evaluate(fehler);
        }
    }
    virtual bool GetSimpleVarReference(std::string &name) {
        if(rohterme.empty()) {
            return false;
        }
        else {
            return rohterme[0]->GetSimpleVarReference(name);
        }
    }

private:
    virtual void DoPrintForError(std::stringstream& ostream, std::vector<unsigned int> &fehler_stellen) {
        ostream << '(';
        if(!rohterme.empty()) {
            typename std::vector<term<Tzahl>*>::iterator it_teil = rohterme.begin();
            (*it_teil)->PrintForError(ostream, fehler_stellen);
            for(it_teil++; it_teil != rohterme.end(); it_teil++) {
                ostream << ", ";
                (*it_teil)->PrintForError(ostream, fehler_stellen);
            }
        }
        ostream << ')';
    }
};

template<class Tzahl>
class expression_value_numbervisitor:
    public boost::static_visitor<>
{
public:
	Tzahl m_zahl;
	bool b_zahl;

    typedef boost::variant<mtypevoid_variant,
            Tzahl
        > expression_value;

    void operator()(mtypevoid_variant& v) {
		b_zahl = false;
    }
    void operator()(Tzahl &c_zahl) {
		m_zahl = c_zahl;
		b_zahl = true;
    }
};

template<class Tzahl>
void PrintForError(term<Tzahl> *c_term, logger_base *c_logger)
{
    std::stringstream sstream;
    std::vector<unsigned int> fehler_stellen;
    c_term->PrintForError(sstream, fehler_stellen);
    std::string fehler_stellen_markierung;
    int vorige_stelle = -1;
    for(std::vector<unsigned int>::iterator it_stelle = fehler_stellen.begin(); it_stelle != fehler_stellen.end(); it_stelle++) {
        int abstand = int(*it_stelle) - vorige_stelle;
        if(abstand > 0) {
            fehler_stellen_markierung.insert(fehler_stellen_markierung.end(), abstand - 1, ' ');
            fehler_stellen_markierung.insert(fehler_stellen_markierung.end(), 1, '^');
            vorige_stelle = (*it_stelle);
        }
        else if(abstand < 0) {
            c_logger->LogError(std::string("FATAL: abstand = ") + Int_To_String(abstand) + ".");
        }
    }
    c_logger->LogError(sstream.str());
    c_logger->LogError(fehler_stellen_markierung);
}

template<class Tzahl>
int ParseExpression(std::string ausdruck_str, logger_base *c_logger, term<Tzahl> **oberterm) // oberterm is only set when it returned 0
{
    typedef grammar1<std::string::const_iterator> grammar1;
    teilausdruck_top tree;

    std::string::const_iterator end = ausdruck_str.end();
    // erstmal einfache Fehler suchen, damit diese verständlich ausgegeben werden können
    unsigned int anzahl_offene_klammern = 0;
    bool fehler = false;
    for(std::string::const_iterator it_zeichen = ausdruck_str.begin(); it_zeichen != end; it_zeichen++) {
        if((*it_zeichen) == '(') {
            anzahl_offene_klammern++;
        }
        else if((*it_zeichen) == ')') {
            if(anzahl_offene_klammern > 0) {
                anzahl_offene_klammern--;
            }
            else {
                fehler = true;
                break;
            }
        }
    }
    if(fehler) {
        c_logger->LogError("Unausgeglichene Klammersetzung.");
        return 2;
    }
    else if(anzahl_offene_klammern > 1) {
        c_logger->LogError("Unausgeglichene Klammersetzung: Noch " + Int_To_String(anzahl_offene_klammern) + " offene Klammern.");
        return 2;
    }
    else if(anzahl_offene_klammern == 1) {
        c_logger->LogError("Unausgeglichene Klammersetzung: Noch eine offene Klammer.");
        return 2;
    }
    else {
        std::string::const_iterator iter = ausdruck_str.begin();
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
            using spirit_encoding::punct; // auch '(', ')'
            using boost::spirit::int_;
            using boost::spirit::float_;
            namespace qlabels = boost::spirit::qi::labels;

            // "\xc3\xb7"
            std::string str;
            std::cout << boost::spirit::qi::phrase_parse(iter, end,
                    lexeme[*( (char_ - space - char_("\xc3=[\\]^{}") - char_('!', '@')) // '_' muss erlaubt sein
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
            term_visitor<Tzahl> c_visitor(oberterm, c_logger, &b_ok);
            boost::apply_visitor(c_visitor, tree);
            if(b_ok) {
                std::stringstream sstream;
                std::vector<unsigned int> fehler_stellen;
                (*oberterm)->PrintForError(sstream, fehler_stellen);
                c_logger->LogInformation(sstream.str());
                return 0;
            }
            else {
                PrintForError(*oberterm, c_logger);
                delete *oberterm;
                return 3;
            }
        }
        else {
            int pos = 0;
            for(std::string::const_iterator it_pos = ausdruck_str.begin(); it_pos != iter; it_pos++) {
                pos++;
            }
            c_logger->LogError(std::string("Fehler beim Parsen nah bei Zeichen ") + Int_To_String(pos) + ":");
            c_logger->LogError(ausdruck_str);
            c_logger->LogError(std::string(pos, ' ') + '^');
            return 1;
        }
    }
}

} // namespace expression_parser
} // namespace stocks_dl

#endif // AUSDRUCK_HPP_INCLUDED
