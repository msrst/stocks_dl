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

#include <cmath>
#include <iostream>
#include <fstream>
#include <iterator>

#include "utils.hpp"
#include "../decl.hpp"

namespace stocks_dl {

uint8_t Hex_To_uint8(char hex1, char hex2)
{
    uint8_t ret = 0;
    if((hex1 > 64) && (hex1 < 71)) //A-F
    {
        hex1 -= 65;
        hex1 += 10;
    }
    else if((hex1 > 96) && (hex1 < 103)) //a - f
    {
        hex1 -= 97;
        hex1 += 10;
    }
    else if((hex1 > 47) && (hex1 < 58)) //0-9
    {
        hex1 -= 48;
    }
    else
    {
        return 0;
    }
    if((hex2 > 64) && (hex2 < 71)) //A-F
    {
        hex2 -= 65;
        hex2 += 10;
    }
    else if((hex2 > 96) && (hex2 < 103)) //a - f
    {
        hex2 -= 97;
        hex2 += 10;
    }
    else if((hex2 > 47) && (hex2 < 58)) //0-9
    {
        hex2 -= 48;
    }
    else
    {
        return 0;
    }
    ret = hex2;
    ret += hex1 * 16;
    return ret;
}

std::string Format_Defaultdate_stdString(long long int datum, char ms_trennzeichen)
{
    time_t sec = datum / 1000;
    long int ms = datum - ((long long int)sec) * 1000;
    char* c_datum = new char[100];
    tm* tmZeit = std::localtime(&sec);
    std::strftime(c_datum, 100, "%d.%m.%Y %X", tmZeit);
    std::string ret(c_datum);
    delete c_datum;
    ret.append(1, ms_trennzeichen);
    if(ms >= 100)
    {
    }
    else if(ms >= 10)
    {
        ret.append("0");
    }
    else
    {
        ret.append("00");
    }
    ret.append(Int_To_String(ms));
    return ret;
}
std::string Format_Sec_Standarddatum_stdString(time_t datum)
{
    char c_datum[100];
    tm* tmZeit = std::localtime(&datum);
    std::strftime(c_datum, 100, "%d.%m.%Y %X", tmZeit);
    return std::string(c_datum);
}

std::string Int_To_String(int64_t i)
{
    if(i == 0)
    {
        return std::string("0");
    }
    else
    {
        std::string ret;
        bool minuszeichen;
        if(i < 0) {
            i = 0 - i;
            minuszeichen = true;
        }
        else {
            minuszeichen = false;
        }
        char zeichen;
        while(i != 0)
        {
            zeichen = static_cast<char>(i % 10) + 48;
            ret.insert(0, 1, zeichen);
            i /= 10;
        }
        if(minuszeichen) {
            ret.insert(0, 1, '-');
        }
        return ret;
    }
}
std::string Int_To_String_Zeros(long long int i, unsigned long int breite)
{
    if(i == 0)
    {
        return std::string(breite, '0');
    }
    else
    {
        std::string ret;
        bool minuszeichen;
        if(i < 0) {
            i = 0 - i;
            minuszeichen = true;
        }
        else {
            minuszeichen = false;
        }
        char zeichen;
        while(i != 0)
        {
            long long int i_durch_10 = i / 10;
            zeichen = i - i_durch_10 * 10 + 48;
            ret.insert(0, 1, zeichen);
            i = i_durch_10;
        }
        if(minuszeichen) {
            int anzahl_vornullen = ((int)breite) - ((int)ret.size()) - 1;
            if(anzahl_vornullen > 0) {
                ret.insert(0, anzahl_vornullen, '0');
            }
            ret.insert(0, 1, '-');
        }
        else {
            int anzahl_vornullen = ((int)breite) - ((int)ret.size());
            if(anzahl_vornullen > 0) {
                ret.insert(0, anzahl_vornullen, '0');
            }
        }
        return ret;
    }
}

//-----------------------------------------------------------------------------------------------------------------------

string_to_int_converter::string_to_int_converter()
{
    Init();
}
void string_to_int_converter::Init()
{
    fehler = 0;
}

long long int string_to_int_converter::ConvertToLLI(std::string rohstring)
{
    return ConvertToLLIFromSIt(rohstring.begin(), rohstring.end());
}
long long int string_to_int_converter::ConvertToLLI_HexOk(std::string rohstring)
{
    return ConvertToLLIFromSIt_HexOk(rohstring.begin(), rohstring.end());
}
long long int string_to_int_converter::ConvertToLLIFromSIt(std::string::iterator begin, std::string::iterator end)
{
    if(begin == end) {
        fehler++;
        return 0;
    }
    else {
        long long int ret = 0;
        std::string::iterator it_zeichen = begin;
        bool negativ;
        if(*it_zeichen == '-') {
            negativ = true;
            it_zeichen++;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else if(*it_zeichen == '+') {
            negativ = false;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else {
            negativ = false;
        }
        for(; it_zeichen != end; it_zeichen++) {
            long long int neue_ziffer = (*it_zeichen) - '0';
            if((neue_ziffer < 0) || (neue_ziffer > 9)) {
                fehler++;
                return 0;
            }
            ret = ret * 10 + neue_ziffer;
        }
        if(negativ) {
            return 0 - ret;
        }
        else {
            return ret;
        }
    }
}
long long int string_to_int_converter::ConvertToLLIFromSIt_HexOk(std::string::iterator begin, std::string::iterator end)
{
    if(begin == end) {
        fehler++;
        return 0;
    }
    else {
        long long int ret = 0;
        if(*begin == '0') {
			begin++;
			if(*begin == 'x') {
				begin++;
				if(begin == end) {
					fehler++;
					return 0;
				}
				for(std::string::iterator it_zeichen = begin; it_zeichen != end; it_zeichen++) {
					ret <<= 4;
					if((*it_zeichen >= '0') && (*it_zeichen <= '9')) {
						ret |= (long long int)((*it_zeichen) - '0') & 0x0f;
					}
					else if((*it_zeichen >= 'A') && (*it_zeichen <= 'F')) {
						ret |= (long long int)((*it_zeichen) - 55) & 0x0f; // 'A' ist Nr. 65
					}
					else if((*it_zeichen >= 'a') && (*it_zeichen <= 'f')) {
						ret |= (long long int)((*it_zeichen) - 87) & 0x0f; // 'a' ist Nr. 97
					}
					else {
						fehler++;
						return 0;
					}
				}
				return ret;
			}
			else {
				begin--;
			}
		}

        std::string::iterator it_zeichen = begin;
        bool negativ;
        if(*it_zeichen == '-') {
            negativ = true;
            it_zeichen++;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else if(*it_zeichen == '+') {
            negativ = false;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else {
            negativ = false;
        }
        for(; it_zeichen != end; it_zeichen++) {
            long long int neue_ziffer = (*it_zeichen) - '0';
            if((neue_ziffer < 0) || (neue_ziffer > 9)) {
                fehler++;
                return 0;
            }
            ret = ret * 10 + neue_ziffer;
        }
        if(negativ) {
            return 0 - ret;
        }
        else {
            return ret;
        }
    }
}
long long int string_to_int_converter::ConvertToLLIFromSIt(std::string::iterator begin, std::string::iterator end, char tausender_zeichen)
{
    if(begin == end) {
        fehler++;
        return 0;
    }
    else {
        long long int ret = 0;
        std::string::iterator it_zeichen = begin;
        bool negativ;
        if(*it_zeichen == '-') {
            negativ = true;
            it_zeichen++;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else if(*it_zeichen == '+') {
            negativ = false;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else {
            negativ = false;
        }
        for(; it_zeichen != end; it_zeichen++) {
            if(*it_zeichen != tausender_zeichen) {
                long long int neue_ziffer = (*it_zeichen) - '0';
                if((neue_ziffer < 0) || (neue_ziffer > 9)) {
                    fehler++;
                    return 0;
                }
                ret = ret * 10 + neue_ziffer;
            }
        }
        if(negativ) {
            return 0 - ret;
        }
        else {
            return ret;
        }
    }
}
double string_to_int_converter::ConvertToDouble(std::string rohstring, char punct_char, char tausender_zeichen)
{
    return ConvertToDoubleFromSIt(rohstring.begin(), rohstring.end(), punct_char, tausender_zeichen);
}
double string_to_int_converter::ConvertToDoubleR(std::string rohstring, char punct_char, char tausender_zeichen)
{
    return ConvertToDoubleRFromSIt(rohstring.begin(), rohstring.end(), punct_char, tausender_zeichen);
}
double string_to_int_converter::ConvertToDoubleFromSIt(std::string::iterator begin, std::string::iterator end, char punct_char, char tausender_zeichen)
{
    if(begin == end) {
        fehler++;
        return 0;
    }
    else {
        double ret = 0;
        std::string::iterator it_zeichen = begin;
        bool negativ;
        if(*it_zeichen == '-') {
            negativ = true;
            it_zeichen++;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else if(*it_zeichen == '+') {
            negativ = false;
            if(it_zeichen == end) {
                fehler++;
                return 0;
            }
        }
        else {
            negativ = false;
        }
        for(; it_zeichen != end; it_zeichen++) {
            if(*it_zeichen == punct_char) {
                double kommastelle_fac = 1;
                for(it_zeichen++; it_zeichen != end; it_zeichen++) {
                    if(*it_zeichen == 'e') {
                        it_zeichen++;
                        if(it_zeichen == end) {
                            fehler++;
                            return 0;
                        }
                        else {
                            if(*it_zeichen == '+') {
                                it_zeichen++;
                                ret *= std::pow(10, ConvertToLLIFromSIt(it_zeichen, end));
                            }
                            else if(*it_zeichen == '-') {
                                it_zeichen++;
                                ret *= std::pow(0.1, ConvertToLLIFromSIt(it_zeichen, end));
                            }
                            else {
                                fehler++;
                                return 0;
                            }
                        }
                        break;
                    }
                    else {
                        kommastelle_fac *= 0.1;
                        ret = ret + ((double)((*it_zeichen) - '0')) * kommastelle_fac;
                    }
                }
                break;
            }
            else if(*it_zeichen == 'e') {
                it_zeichen++;
                if(it_zeichen == end) {
                    fehler++;
                    return 0;
                }
                else {
                    if(*it_zeichen == '+') {
                        it_zeichen++;
                        ret *= std::pow(10, ConvertToLLIFromSIt(it_zeichen, end));
                    }
                    else if(*it_zeichen == '-') {
                        it_zeichen++;
                        ret *= std::pow(0.1, ConvertToLLIFromSIt(it_zeichen, end));
                    }
                    else {
                        fehler++;
                        return 0;
                    }
                }
                break;
            }
            else if(*it_zeichen != tausender_zeichen) {
                double neue_ziffer = (*it_zeichen) - '0';
                if((neue_ziffer < 0) || (neue_ziffer > 9)) {
                    fehler++;
                    return 0;
                }
                ret = ret * 10 + neue_ziffer;
            }
        }
        if(negativ) {
            return 0 - ret;
        }
        else {
            return ret;
        }
    }
}
double string_to_int_converter::ConvertToDoubleRFromSIt(std::string::iterator begin, std::string::iterator end, char punct_char, char tausender_zeichen)
{
	std::string::iterator it_slash;
	for(it_slash = begin; it_slash != end; it_slash++) {
		if(*it_slash == '/') {
			break;
		}
	}
	if(it_slash == end) {
		return ConvertToDoubleFromSIt(begin, end, punct_char, tausender_zeichen);
	}
	else {
		long long int zaehler = ConvertToLLIFromSIt(begin, it_slash);
		it_slash++;
		long long int nenner = ConvertToLLIFromSIt(it_slash, end);
		if(nenner == 0) {
			fehler++;
			return 0;
		}
		else {
			return double(zaehler) / nenner;
		}
	}
}
bool string_to_int_converter::MistakeHappened()
{
    return (fehler != 0);
}

//-----------------------------------------------------------------------------------------------------------------------

string_to_int_limited_converter::string_to_int_limited_converter()
{
    Init();
}
void string_to_int_limited_converter::Init()
{
    limitfehler = 0;
}

long long int string_to_int_limited_converter::ConvertToLLI_Limited(std::string rohstring, long long int min, long long int max)
{
    return ConvertToLLIFromSIt_Limited(rohstring.begin(), rohstring.end(), min, max);
}
long long int string_to_int_limited_converter::ConvertToLLI_HexOk_Limited(std::string rohstring, long long int min, long long int max)
{
    return ConvertToLLIFromSIt_HexOk_Limited(rohstring.begin(), rohstring.end(), min, max);
}
long long int string_to_int_limited_converter::ConvertToLLIFromSIt_Limited(std::string::iterator begin, std::string::iterator end, long long int min, long long int max)
{
    long long int pruef = ConvertToLLIFromSIt(begin, end);
    if(pruef < min) {
        pruef = min;
        limitfehler++;
    }
    else if(pruef > max) {
        pruef = max;
        limitfehler++;
    }
    return pruef;
}
long long int string_to_int_limited_converter::ConvertToLLIFromSIt_HexOk_Limited(std::string::iterator begin, std::string::iterator end, long long int min, long long int max)
{
    long long int pruef = ConvertToLLIFromSIt_HexOk(begin, end);
    if(pruef < min) {
        pruef = min;
        limitfehler++;
    }
    else if(pruef > max) {
        pruef = max;
        limitfehler++;
    }
    return pruef;
}
double string_to_int_limited_converter::ConvertToDouble_Limited(std::string rohstring, double min, double max, char punct_char, char tausender_zeichen)
{
    double pruef = ConvertToDouble(rohstring, punct_char, tausender_zeichen);
    if(pruef < min) {
        pruef = min;
        limitfehler++;
    }
    else if(pruef > max) {
        pruef = max;
        limitfehler++;
    }
    return pruef;
}
double string_to_int_limited_converter::ConvertToDoubleR_Limited(std::string rohstring, double min, double max, char punct_char, char tausender_zeichen)
{
    double pruef = ConvertToDoubleR(rohstring, punct_char, tausender_zeichen);
    if(pruef < min) {
        pruef = min;
        limitfehler++;
    }
    else if(pruef > max) {
        pruef = max;
        limitfehler++;
    }
    return pruef;
}

bool string_to_int_limited_converter::LimitMistakeHappened()
{
    return (limitfehler != 0);
}

//-----------------------------------------------------------------------------------------------------------------------

bool GetFileString(std::string dateiname, std::string* value)
{
    std::ifstream datei_istream;
    datei_istream.open(dateiname.c_str(), std::ios_base::in);
    if(datei_istream) {
        datei_istream.unsetf(std::ios::skipws); // No white space skipping!
        value->clear();
        std::copy(
            std::istream_iterator<char>(datei_istream),
            std::istream_iterator<char>(), // allgemeiner End-Iterator
            std::back_inserter(*value)); // dieser Iterator ist keine Referenz. Stattdessen ruft er bei einer
                            // Zuweisung (*value).push_back(.) auf und macht nichts, wenn it++ aufgerufen wird.
        return true;
    }
    else {
        return false;
    }
}
bool SetFileString(std::string dateiname, const std::string &value)
{
	std::ofstream datei_ostream;
    datei_ostream.open(dateiname.c_str(), std::ios_base::out | std::ios_base::binary);
    if(datei_ostream) {
		datei_ostream.write(value.c_str(), value.length());
        return true;
    }
    else {
        return false;
    }
}

bool str_begins_with(const std::string &str, const std::string &begin)
{
	if(str.size() >= begin.size()) {
		for(unsigned int i1 = 0; i1 < begin.size(); i1++) {
			if(str[i1] != begin[i1]) {
				return false;
			}
		}
		return true;
	}
	else {
		return false;
	}
}
bool str_ends_with(const std::string &str, const std::string &end)
{
	if(str.size() >= end.size()) {
		unsigned int offset = str.size() - end.size();
		for(unsigned int i1 = 0; i1 < end.size(); i1++) {
			if(str[i1+offset] != end[i1]) {
				return false;
			}
		}
		return true;
	}
	else {
		return false;
	}
}
void str_replace_char(std::string &str, char suchen, std::string ersetzen)
{
	for(unsigned int izeichen = 0; izeichen < str.length(); ) {
		if(str[izeichen] == suchen) {
			str.replace(izeichen, 1, ersetzen);
			izeichen += ersetzen.length();
		}
		else {
			izeichen++;
		}
	}
}
std::vector<std::string> str_split(const std::string &str, const std::string &split)
{
	std::vector<std::string> splits;

	if(split.empty()) {
		std::cout << "Warnung: str_split: split = \"\"." << std::endl;
		splits.push_back(split);
		return splits;
	}

	std::size_t splitstr_begin = 0;
	do {
		std::size_t splitstr_end = str.find(split, splitstr_begin);
		if(splitstr_end == std::string::npos) {
			splits.push_back(str.substr(splitstr_begin, str.size() - splitstr_begin)); // Den Rest anhängen
			return splits;
		}
		else {
			splits.push_back(str.substr(splitstr_begin, splitstr_end - splitstr_begin)); // substr(pos, laenge)
			splitstr_begin = splitstr_end + split.size();
		}
	} while(splitstr_begin != str.size());
	return splits;
}
#define FN_STR_ISWHITE(zeichen) ((zeichen == ' ') || (zeichen == '\t') || (zeichen == '\n') || (zeichen == '\r'))
std::string str_remove_sidewhites(const std::string &text)
{
	if(text.empty()) {
		return text;
	}
	unsigned int begin, last;
	for(begin = 0; begin < text.length(); begin++) {
		if(!FN_STR_ISWHITE(text[begin])) {
			break;
		}
	}
	for(last = text.length() - 1; last > 0; last--) {
		if(!FN_STR_ISWHITE(text[last])) {
			break;
		}
	}
	if((last == 0) && FN_STR_ISWHITE(text[0])) {
		return std::string();
	}
	return text.substr(begin, last - begin + 1);
}
std::string str_to_upper(std::string str)
{
	bool b_umlaut = false;
	for(std::string::iterator it_zeichen = str.begin(); it_zeichen != str.end(); it_zeichen++) {
		if((*it_zeichen >= 'a') && (*it_zeichen <= 'z')) {
			*it_zeichen = *it_zeichen - 'a' + 'A';
		}
		else if(*it_zeichen == '\xc3') {
			b_umlaut = true;
		}
		else if(b_umlaut) {
			if(*it_zeichen == '\xa4') { // ä
				*it_zeichen = '\x84';
			}
			else if(*it_zeichen == '\xb6') { // ö
				*it_zeichen = '\x96';
			}
			else if(*it_zeichen == '\xbc') { // ü
				*it_zeichen = '\x9c';
			}
			b_umlaut = false;
		}
	}
	return str;
}

std::string vectorstring_unsplit(const std::vector<std::string> &array, char zeichen)
{
	if(array.empty()) {
		return std::string();
	}
	std::string ret = array[0];
	for(unsigned int i1 = 1; i1 < array.size(); i1++) {
		ret.append(1, zeichen);
		ret.append(array[i1]);
	}
	return ret;
}

/*
int bitap_suche(char* text, char* muster, unsigned int ltext, unsigned int lmuster)
{
    std::vector<bool> R(lmuster + 1);
    R[0] = 1; // Das erste Element ist nur Dummy und sorgt dafür, dass nach dem Bitshift R[1] mit 1 initialisiert wird
    for(int i = 0; i < ltext; i=i+1){
        for(int k = lmuster; k > 0; k -= 1){
            R[k] = R[k-1]; // Bitshift
            if(text[i] == muster[k-1]){
                R[k] = R[k] & 1;
            }
            else{
                R[k] = R[k] & 0;
            }
        }
        if(R[lmuster] == 1){
            int position = (i - lmuster) + 1;
            return position;
        }
    }
	return -1;
}*/
// ä: 0x00e4, Ä: 0x00c4, ö: 0x00f6, Ö: 0x00d6, ü: 0x00fc, Ü: 0x00dc, ß: 0x00df
void str_find_caseinsensitive_mangle(std::vector<uint32_t> &str, bool leerz)
{
	// ss wird zu ß gemacht, ae und Ä zu ä usw.
	for(unsigned int i1 = 1; i1 < str.size(); i1++) {
		if( ((str[i1-1] == 's') || (str[i1-1] == 'S')) && ((str[i1] == 's') || (str[i1] == 'S')) ) {
			str[i1-1] = 0x00df;
			str.erase(str.begin() + i1);
		}
		else if( ((str[i1-1] == 'a') || (str[i1-1] == 'A')) && ((str[i1] == 'e') || (str[i1] == 'E')) ) {
			str[i1-1] = 0x00e4;
			str.erase(str.begin() + i1);
		}
		else if( ((str[i1-1] == 'o') || (str[i1-1] == 'O')) && ((str[i1] == 'e') || (str[i1] == 'E')) ) {
			str[i1-1] = 0x00f6;
			str.erase(str.begin() + i1);
		}
		else if( ((str[i1-1] == 'U') || (str[i1-1] == 'U')) && ((str[i1] == 'e') || (str[i1] == 'E')) ) {
			str[i1-1] = 0x00fc;
			str.erase(str.begin() + i1);
		}
	}
	if(leerz) {
		for(unsigned int i1 = 0; i1 < str.size(); i1++) {
			if((str[i1] == '-') || (str[i1] == '_')) {
				str[i1] = ' ';
			}
			else if(str[i1] == 0x00c4) {
				str[i1] = 0x00e4;
			}
			else if(str[i1] == 0x00d6) {
				str[i1] = 0x00f6;
			}
			else if(str[i1] == 0x00dc) {
				str[i1] = 0x00fc;
			}
		}
	}
	else {
		for(unsigned int i1 = 0; i1 < str.size(); i1++) {
			if(str[i1] == 0x00c4) {
				str[i1] = 0x00e4;
			}
			else if(str[i1] == 0x00dc) {
				str[i1] = 0x00f6;
			}
			else if(str[i1] == 0x00dc) {
				str[i1] = 0x00fc;
			}
		}
	}
}
size_t str_find_caseinsensitive(const std::string &text, const std::string &muster, bool leerz)
{
	std::vector<uint32_t> textu = StringToUTF8String(text.begin(), text.end());
	std::vector<uint32_t> musteru = StringToUTF8String(muster.begin(), muster.end());

	str_find_caseinsensitive_mangle(textu, leerz);
	str_find_caseinsensitive_mangle(musteru, leerz);
	if(musteru.empty()) {
		return std::string::npos;
	}

	std::vector<bool> R(musteru.size() + 1);
    R[0] = 1; // Das erste Element ist nur Dummy und sorgt dafür, dass nach dem Bitshift R[1] mit 1 initialisiert wird
    for(size_t i = 0; i < textu.size(); i=i+1){
        for(int k = musteru.size(); k > 0; k -= 1){
            R[k] = R[k-1]; // Bitshift
            bool equal = false;
            if(textu[i] == musteru[k-1]){
                equal = true;
            }
            else if((textu[i] >= 'A') && (textu[k-1] <= 'Z')) {
				if(textu[i] - 'A' + 'a' == musteru[k-1]) {
					equal = true;
				}
			}
            else if((textu[i] >= 'a') && (textu[k-1] <= 'z')) {
				if(textu[i] - 'a' + 'A' == musteru[k-1]) {
					equal = true;
				}
			}
			R[k] = R[k] & equal;
        }
        if(R[musteru.size()] == 1) {
            int position = (i - musteru.size()) + 1;
            return position;
        }
    }
	return std::string::npos;
}
bool str_begins_with_caseinsensitive(const std::string &text, const std::string &muster, bool word_end, bool leerz)
{
	std::vector<uint32_t> textu = StringToUTF8String(text.begin(), text.end());
	std::vector<uint32_t> musteru = StringToUTF8String(muster.begin(), muster.end());

	str_find_caseinsensitive_mangle(textu, leerz);
	str_find_caseinsensitive_mangle(musteru, leerz);
	if(musteru.empty()) {
		return false;
	}

    if(muster.size() > textu.size()) {
		return false;
	}
    for(size_t i = 0; i < musteru.size(); i=i+1){
		bool equal = false;
		if(textu[i] == musteru[i]){
			equal = true;
		}
		else if((textu[i] >= 'A') && (textu[i] <= 'Z')) {
			if(textu[i] - 'A' + 'a' == musteru[i]) {
				equal = true;
			}
		}
		else if((textu[i] >= 'a') && (textu[i] <= 'z')) {
			if(textu[i] - 'a' + 'A' == musteru[i]) {
				equal = true;
			}
		}

		if(!equal) {
			return false;
		}
    }
    if(word_end && (textu.size() > musteru.size())) {
		if(textu[musteru.size()] != ' ') {
			return false;
		}
	}
	return true;
}
size_t str_find_without_whites(const std::string &text, const std::string &muster)
{
	std::string muster_neu;
	for(unsigned int i1 = 0; i1 < muster.length(); i1++) {
		if((muster[i1] != ' ') && (muster[i1] != '\t') && (muster[i1] != '\n') && (muster[i1] != '\r')) {
			muster_neu.append(1, muster[i1]);
		}
	}
	std::vector<bool> R(muster_neu.length() + 1);
    R[0] = 1; // Das erste Element ist nur Dummy und sorgt dafür, dass nach dem Bitshift R[1] mit 1 initialisiert wird
    for(unsigned int i = 0; i < text.length(); i++){
		if((text[i] != ' ') && (text[i] != '\t') && (text[i] != '\n') && (text[i] != '\r')) {
			for(int k = muster_neu.length(); k > 0; k--){
				R[k] = R[k-1]; // Bitshift
				if(text[i] == muster_neu[k-1]){
					R[k] = R[k] & 1;
				}
				else{
					R[k] = R[k] & 0;
				}
			}
			if(R[muster_neu.length()] == 1){
				int position = i;
				for(unsigned int i1 = 1; i1 < muster_neu.length(); i1++) {
					position--;
					while(FN_STR_ISWHITE(text[position])) {
						position--;
					}
				}
				return position;
			}
		}
    }
	return std::string::npos;
}

//-----------------------------------------------------------------------------------------------------------------------

// 1xxxxxxx
#define UNICODE_UNNORMAL_MASK (0x80)
// 11111xxx
#define UNICODE_4ZEICHEN_MASK (0xF8)
// 11110000
#define UNICODE_4ZEICHEN_KORREKT (0xF0)
// 00000111
#define UNICODE_4ZEICHEN_BEGINNDATAMASK (0x07)
// 1111xxxx
#define UNICODE_3ZEICHEN_MASK (0xF0)
// 11100000
#define UNICODE_3ZEICHEN_KORREKT (0xE0)
// 00001111
#define UNICODE_3ZEICHEN_BEGINNDATAMASK (0x0F)
// 111xxxxx
#define UNICODE_2ZEICHEN_MASK (0xE0)
// 11000000
#define UNICODE_2ZEICHEN_KORREKT (0xC0)
// 00011111
#define UNICODE_2ZEICHEN_BEGINNDATAMASK (0x1F)
// 11000000
#define UNICODE_NEXTZEICHEN_PRUEFMASK (0xC0)
// 10000000
#define UNICODE_NEXTZEICHEN_KORREKT (0x80)
// 00111111
#define UNICODE_NEXTZEICHEN_DATAMASK (0x3F)

void utf8_parser::Start()
{
    char zeichen;
    unsigned int neues_zeichen;
    zeichen = GetNextZeichen();
    while(!(EndeErreicht())) {
        if((zeichen & UNICODE_UNNORMAL_MASK) == UNICODE_UNNORMAL_MASK) {
            if((zeichen & UNICODE_4ZEICHEN_MASK) == UNICODE_4ZEICHEN_KORREKT) {
                       //std::cout << "4zeichen" << std::endl;
                neues_zeichen = (zeichen & UNICODE_4ZEICHEN_BEGINNDATAMASK)  << 18;
                if(!(EndeErreicht())) {
                    zeichen = GetNextZeichen();
                    if((zeichen & UNICODE_NEXTZEICHEN_PRUEFMASK) == UNICODE_NEXTZEICHEN_KORREKT) {
                        neues_zeichen |= (zeichen & UNICODE_NEXTZEICHEN_DATAMASK) << 12;
                        if(!(EndeErreicht())) {
                            zeichen = GetNextZeichen();
                            if((zeichen & UNICODE_NEXTZEICHEN_PRUEFMASK) == UNICODE_NEXTZEICHEN_KORREKT) {
                                neues_zeichen |= (zeichen & UNICODE_NEXTZEICHEN_DATAMASK) << 6;
                                if(!(EndeErreicht())) {
                                    zeichen = GetNextZeichen();
                                    if((zeichen & UNICODE_NEXTZEICHEN_PRUEFMASK) == UNICODE_NEXTZEICHEN_KORREKT) {
                                        neues_zeichen |= (zeichen & UNICODE_NEXTZEICHEN_DATAMASK);
                                        NeuesZeichen(neues_zeichen);
                                    }
                                    else {
                                        UTF8Fehler();
                                    }
                                }
                            }
                            else {
                                UTF8Fehler();
                            }
                        }
                    }
                    else {
                        UTF8Fehler();
                        break;
                    }
                }
            }
            else if((zeichen & UNICODE_3ZEICHEN_MASK) == UNICODE_3ZEICHEN_KORREKT) {
                       //std::cout << "3zeichen" << std::endl;
                neues_zeichen = (zeichen & UNICODE_3ZEICHEN_BEGINNDATAMASK) << 12;
                if(!(EndeErreicht())) {
                    zeichen = GetNextZeichen();
                    if((zeichen & UNICODE_NEXTZEICHEN_PRUEFMASK) == UNICODE_NEXTZEICHEN_KORREKT) {
                        neues_zeichen |= (zeichen & UNICODE_NEXTZEICHEN_DATAMASK) << 6;
                        if(!(EndeErreicht())) {
                            zeichen = GetNextZeichen();
                            if((zeichen & UNICODE_NEXTZEICHEN_PRUEFMASK) == UNICODE_NEXTZEICHEN_KORREKT) {
                                neues_zeichen |= (zeichen & UNICODE_NEXTZEICHEN_DATAMASK);
                                NeuesZeichen(neues_zeichen);
                            }
                            else {
                                UTF8Fehler();
                            }
                        }
                    }
                    else {
                        UTF8Fehler();
                        break;
                    }
                }
            }
            else if((zeichen & UNICODE_2ZEICHEN_MASK) == UNICODE_2ZEICHEN_KORREKT) {
                       //std::cout << "2zeichen" << std::endl;
                neues_zeichen = (zeichen & UNICODE_2ZEICHEN_BEGINNDATAMASK) << 6;
                if(!(EndeErreicht())) {
                    zeichen = GetNextZeichen();
                    if((zeichen & UNICODE_NEXTZEICHEN_PRUEFMASK) == UNICODE_NEXTZEICHEN_KORREKT) {
                        neues_zeichen |= (zeichen & UNICODE_NEXTZEICHEN_DATAMASK);
                        NeuesZeichen(neues_zeichen);
                    }
                    else {
                        UTF8Fehler();
                        break;
                    }
                }
            }
            else {
                UTF8Fehler();
                break;
            }
        }
        else {
            NeuesZeichen(zeichen);
        }
        zeichen = GetNextZeichen();
    }
}

class utf8_string_parser:
    public utf8_parser
{
private:
    std::string::const_iterator m_it_end;
    std::string::const_iterator it_pos;
    std::vector<uint32_t> m_string;
    bool b_fehler;
    bool b_ende_erreicht;

public:
    utf8_string_parser(std::string::const_iterator it_begin, std::string::const_iterator it_end);

    virtual bool EndeErreicht();
    virtual char GetNextZeichen();
    virtual void NeuesZeichen(uint32_t zeichen);
    virtual void UTF8Fehler();

    bool MistakeHappened()
    {
        return b_fehler;
    }
    std::vector<uint32_t> GetString()
    {
        return m_string;
    }

    void DoStart()
    {
        Start();
    }
};
utf8_string_parser::utf8_string_parser(std::string::const_iterator it_begin, std::string::const_iterator it_end)
{
    m_it_end = it_end;
    it_pos = it_begin;
    b_fehler = false;
    b_ende_erreicht = false;
}
bool utf8_string_parser::EndeErreicht()
{
    return b_ende_erreicht;
}
char utf8_string_parser::GetNextZeichen()
{
    if(it_pos == m_it_end) {
        b_ende_erreicht = true;
        return 0;
    }
    else {
        return *(it_pos++);
    }
}
void utf8_string_parser::NeuesZeichen(uint32_t zeichen)
{
    m_string.push_back(zeichen);
}
void utf8_string_parser::UTF8Fehler()
{
    b_fehler = true;
}

unsigned int UTF8Laenge(std::string str)
{
    unsigned int laenge = 0;
    for(std::string::iterator it_char = str.begin(); it_char != str.end(); it_char++) {
        if((((*it_char) & 0x80) == 0x00) || (((*it_char) & 0xc0) == 0xc0)) { // 0xf0 = 1000 0000; 0xc0 = 1100 0000
            laenge++;
        }
    }
    return laenge;
}
std::vector<uint32_t> StringToUTF8String(std::string::const_iterator it_begin, std::string::const_iterator it_end, bool* fehler_passiert)
{
    utf8_string_parser parser(it_begin, it_end);
    parser.DoStart();
    if(fehler_passiert) {
        *fehler_passiert = parser.MistakeHappened();
    }
    return parser.GetString();
}

//-----------------------------------------------------------------------------------------------------------------------

uint64_t GetTime_Microseconds()
{
    // clock_gettime kommt von pthread -> bei Windows nicht benutzbar
   #ifdef MPROGRAMM_WINDOWS
    //return clock() * 1000;
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
   #else
	struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)(ts.tv_sec))*1000000 + ((uint64_t)(ts.tv_nsec/1000));
   #endif
}
long long int GetTime_Milliseconds()
{
    // clock_gettime kommt von pthread -> bei Windows nicht benutzbar
   #ifdef MPROGRAMM_WINDOWS
    //return clock() * 1000;
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   #else
	struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((long long int)(ts.tv_sec))*1000 + ((long long int)(ts.tv_nsec/1000000));
   #endif
}

template<typename Tstr>
std::string FormatToHex(Tstr string, bool small_caps, bool space)
{
    std::string str_ret;
    typename Tstr::iterator it_zeichen = string.begin();
    if(string.empty()) {
        return str_ret;
    }
    if(small_caps) {
        uint8_t nibble = (((*it_zeichen) & 0xf0) >> 4);
        if(nibble > 9) {
            str_ret.append(1, 'a' - 10 + nibble);
        }
        else {
            str_ret.append(1, '0' + nibble);
        }
        nibble = (*it_zeichen) & 0x0f;
        if(nibble > 9) {
            str_ret.append(1, 'a' - 10 + nibble);
        }
        else {
            str_ret.append(1, '0' + nibble);
        }
        for(it_zeichen++; it_zeichen != string.end(); it_zeichen++) {
            if(space) {
                str_ret.append(1, ' ');
            }
            nibble = (((*it_zeichen) & 0xf0) >> 4);
            if(nibble > 9) {
                str_ret.append(1, 'a' - 10 + nibble);
            }
            else {
                str_ret.append(1, '0' + nibble);
            }
            nibble = (*it_zeichen) & 0x0f;
            if(nibble > 9) {
                str_ret.append(1, 'a' - 10 + nibble);
            }
            else {
                str_ret.append(1, '0' + nibble);
            }
        }
    }
    else {
        uint8_t nibble = (((*it_zeichen) & 0xf0) >> 4);
        if(nibble > 9) {
            str_ret.append(1, 'A' - 10 + nibble);
        }
        else {
            str_ret.append(1, '0' + nibble);
        }
        nibble = (*it_zeichen) & 0x0f;
        if(nibble > 9) {
            str_ret.append(1, 'A' - 10 + nibble);
        }
        else {
            str_ret.append(1, '0' + nibble);
        }
        for(it_zeichen++; it_zeichen != string.end(); it_zeichen++) {
            if(space) {
                str_ret.append(1, ' ');
            }
            nibble = (((*it_zeichen) & 0xf0) >> 4);
            if(nibble > 9) {
                str_ret.append(1, 'A' - 10 + nibble);
            }
            else {
                str_ret.append(1, '0' + nibble);
            }
            nibble = (*it_zeichen) & 0x0f;
            if(nibble > 9) {
                str_ret.append(1, 'A' - 10 + nibble);
            }
            else {
                str_ret.append(1, '0' + nibble);
            }
        }
    }
    return str_ret;
}
// Instantiations
template std::string FormatToHex(std::string string, bool small_caps, bool space);
template std::string FormatToHex(std::basic_string<uint8_t> string, bool small_caps, bool space);

} // namespace stocks_dl
