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

#ifndef STOCKS_DL_TNULL_HPP_INCLUDED
#define STOCKS_DL_TNULL_HPP_INCLUDED

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "utils.hpp"
#include "../decl.hpp"

namespace stocks_dl {

/* tnull
 * intelligent variable that normally acts like its type but also can be unset.
 * 
 * Examples:
 * tnull<double> my_ndouble = 1234.5;
 * std::cout << my_ndouble << std::endl; // prints 1234.5
 * 
 * my_ndouble.reset(); // now it is unset
 * if(!my_ndouble) { // !my_ndouble evaluates true because it is unset
 *     my_ndouble = 5;
 * }
 * 
 * double my_simple_double = *my_ndouble; // the value can be obtained with asterisk
 * 
 * tnull<double> my_ndouble2 = my_ndouble * my_simple_double; // my_ndouble2 is set if my_ndouble is also set
 */
template <typename T>
class tnull
{
public:
	boost::shared_ptr<T> ptr;
	
	tnull() {}
	tnull(const T &val):
		ptr(boost::make_shared<T>(val)) {}
	
	T& operator*() {
		if(!ptr)
			std::cout << "Warning: ptr == NULL" << std::endl;
		return *ptr;
	}
	const T& operator*() const {
		if(!ptr)
			std::cout << "Warning: ptr == NULL" << std::endl;
		return *ptr;
	}
	bool is_set() const {
		if(ptr)
			return true;
		else
			return false;
	}
	explicit operator bool() const { // explicit -> double b = tnull_zahl does not evaluate (whithout explicit, it would evaluate to 0 or 1)
		return is_set();
	}
	const T* getptr() const { // returns a ptr to the internal value
		return ptr.get();
	}
	T* getptr() {
		return ptr.get();
	}
	
	tnull<T> &set(const T &val) {
		ptr = boost::make_shared<T>(val);
		return *this;
	}
	tnull<T> &operator= (const T &val) {
		return set(val);
	}
	void reset() {
		ptr.reset();
	}
	
	template <typename Tstream>
	friend Tstream &operator<<(Tstream &c_stream, const tnull<T> &val) {
		if(val)
			c_stream << *val;
		//else
		//	c_stream << "NULL";
		return c_stream;
	}
	
	friend tnull<T> operator*(const tnull<T> &a, const tnull<T> &b) {
		if(a && b)
			return (*a) * (*b);
		else
			return tnull<T>();
	}
	friend tnull<T> operator*(const tnull<T> &a, const T &b) {
		if(a)
			return (*a) * b;
		else
			return tnull<T>();
	}
	friend tnull<T> operator*(const T &a, const tnull<T> &b) {
		if(b)
			return a * (*b);
		else
			return tnull<T>();
	}
	tnull<T> operator*=(const tnull<T> &b) {
		*this = (*this) * b;
		return *this;
	}
	tnull<T> operator*=(const T &b) {
		*this = (*this) * b;
		return *this;
	}
	
	friend tnull<T> operator/(const tnull<T> &a, const tnull<T> &b) {
		if(a && b)
			return (*a) / (*b);
		else
			return tnull<T>();
	}
	friend tnull<T> operator/(const tnull<T> &a, const T &b) {
		if(a)
			return (*a) / b;
		else
			return tnull<T>();
	}
	friend tnull<T> operator/(const T &a, const tnull<T> &b) {
		if(b)
			return a / (*b);
		else
			return tnull<T>();
	}
	tnull<T> operator/=(const tnull<T> &b) {
		*this = (*this) / b;
		return *this;
	}
	tnull<T> operator/=(const T &b) {
		*this = (*this) / b;
		return *this;
	}
};

typedef tnull<double> tnull_double;
class tnull_double_adv:
	public tnull_double
{
public:
	tnull_double& operator=(const double &val) {
		return set(val);
	}

	void FromString(string_to_int_converter &convertobj, std::string str, char komma_zeichen = '.', char tausender_zeichen = ',') {
		int fehler = convertobj.GetError();
		double val = convertobj.ConvertToDouble(str, komma_zeichen, tausender_zeichen);
		if(convertobj.GetError() == fehler) {
			*((tnull<double>*)this) = val;
		}
	}
	void FromString_Limited(string_to_int_limited_converter &convertobj, std::string str, double min, double max, char komma_zeichen = '.', char tausender_zeichen = ',') {
		int fehler = convertobj.GetError(), limitfehler = convertobj.GetLimiterror();
		double val = convertobj.ConvertToDouble_Limited(str, min, max, komma_zeichen, tausender_zeichen);
		if((convertobj.GetError() == fehler) && (convertobj.GetLimiterror() == limitfehler)) {
			*((tnull<double>*)this) = val;
		}
	}
	void FromCString(string_to_int_converter &convertobj, const char *cstr, unsigned int laenge, char komma_zeichen = '.', char tausender_zeichen = ',') {
		if(cstr) {
			int fehler = convertobj.GetError();
			double val = convertobj.ConvertToDouble(std::string(cstr, laenge), komma_zeichen, tausender_zeichen);
			if(convertobj.GetError() == fehler) {
				*((tnull<double>*)this) = val;
			}
		}
	}
};

} // namespace stocks_dl

#endif // STOCKS_DL_TNULL_HPP_INCLUDED
