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

#ifndef STOCKS_DL_UTILS_HPP_INCLUDED
#define STOCKS_DL_UTILS_HPP_INCLUDED

#include <inttypes.h>
#include <string>
#include <vector>
#include <ctime>

#define DATE_MIN_YEAR (1800)
#define DATE_MAX_YEAR (2200)

namespace stocks_dl {

uint8_t Hex_To_uint8(char hex1, char hex2); // Upper and lower chars are possible

std::string Format_Defaultdate_stdString(long long int datum, char ms_trennzeichen = ':');
std::string Format_Sec_Standarddatum_stdString(time_t datum);

std::string Int_To_String(int64_t i);
// Before an optional minus sign zeros are inserted so that the width equals to the width parameter.
std::string Int_To_String_Zeros(long long int i, unsigned long int width);

// suffix _HexOk: "0x" at the begin is discarded and only the rest of the string is read as hex format.
// suffix R: also fractions possible (p. e. 5/3).
class string_to_int_converter
{
private:
    int fehler;

public:
    string_to_int_converter();
private:
    void Init();
public:

    long long int ConvertToLLI(std::string rohstring);
    long long int ConvertToLLI_HexOk(std::string rohstring);
    long long int ConvertToLLIFromSIt(std::string::iterator begin, std::string::iterator end);
    long long int ConvertToLLIFromSIt_HexOk(std::string::iterator begin, std::string::iterator end);
    long long int ConvertToLLIFromSIt(std::string::iterator begin, std::string::iterator end, char tausender_zeichen);
    // Possible values when punct_char is '.': "5e-3", "5.2e+3", ".e-4" and "+5".
    double ConvertToDoubleFromSIt(std::string::iterator begin, std::string::iterator end, char punct_char = '.', char tausender_zeichen = ',');
    double ConvertToDoubleRFromSIt(std::string::iterator begin, std::string::iterator end, char punct_char = '.', char tausender_zeichen = ',');
    double ConvertToDouble(std::string rohstring, char punct_char = '.', char tausender_zeichen = ',');
    double ConvertToDoubleR(std::string rohstring, char punct_char = '.', char tausender_zeichen = ',');

    bool MistakeHappened();
    int GetError() {
		return fehler;
	}
};
class string_to_int_limited_converter:
    public string_to_int_converter
{
private:
    int limitfehler;

public:
    string_to_int_limited_converter();
private:
    void Init();
public:

    long long int ConvertToLLI_Limited(std::string rohstring, long long int min, long long int max);
    long long int ConvertToLLI_HexOk_Limited(std::string rohstring, long long int min, long long int max);
    long long int ConvertToLLIFromSIt_Limited(std::string::iterator begin, std::string::iterator end, long long int min, long long int max);
    long long int ConvertToLLIFromSIt_HexOk_Limited(std::string::iterator begin, std::string::iterator end, long long int min, long long int max);
    double ConvertToDouble_Limited(std::string rohstring, double min, double max, char punct_char = '.', char tausender_zeichen = ',');
    double ConvertToDoubleR_Limited(std::string rohstring, double min, double max, char punct_char = '.', char tausender_zeichen = ',');

    bool LimitMistakeHappened();
    int GetLimiterror() {
		return limitfehler;
	}
};

bool GetFileString(std::string dateiname, std::string* value); // value is overwritten, not appended.
bool SetFileString(std::string dateiname, const std::string &value);

char* StdStringToNewChar(std::string stdstr); // Falls in einer Funktion mehrere C-Strings angegeben werden
                                        // müssen, ist es sicherer, für jeden ein neues char-Array zu erstellen,
                                        // weil vielleicht ein weiterer Aufruf von std::string::c_str() das
                                        // vorige c_str()-Ergebnis überschreibt.
                                        // Der Ergebnisstring wird mit \0 abgeschlossen.
bool str_begins_with(const std::string &str, const std::string &begin);
bool str_ends_with(const std::string &str, const std::string &end);
// chars durch chars ersetzen geht mit algorithm: std::replace
void str_replace_char(std::string &str, char suchen, std::string ersetzen);
std::vector<std::string> str_split(const std::string &str, const std::string &split); // The returned vector
										// contains at least one string (that is empty), even if str is empty.
std::string str_to_upper(std::string str); // auch ä, ö, ü

std::string vectorstring_unsplit(const std::vector<std::string> &array, char zeichen);

class utf8_parser
{
protected:
    virtual bool EndeErreicht() = 0; // EndeErreicht() soll erst dann true sein, wenn das Ende erreicht worden ist (also
                              // nachdem nach dem letzen Zeichen einmal GetNextZeichen() aufgerufen wurde). Das ist so,
                              // weil z. B. beim Lesen von Dateien erst file.eof() true ist, wenn das Ende erreicht worden
                              // ist.
    virtual char GetNextZeichen() = 0;
    virtual void NeuesZeichen(uint32_t zeichen) = 0;
    virtual void UTF8Fehler() {} // On error, the incorrect chars are skipped.

    void Start();
};

unsigned int UTF8Laenge(std::string str);
std::vector<uint32_t> StringToUTF8String(std::string::const_iterator it_begin, std::string::const_iterator it_end, bool* fehler_passiert = NULL); // Incorrect
							// chars are skipped, no aborting.

// Also ä, ö, ü can be upper- / lowercase.
// Dafür werden text und muster vorher mit fn_StringToUTF8String konvertiert. Auch ß, ss und SS
// dürfen vertauscht werden.
// If leerz is set, '-', '_' und ' ' are treated as equal.
size_t str_find_caseinsensitive(const std::string &text, const std::string &muster, bool leerz = true); // When sucessful,
					// the function returns the position in the UTF8 string (not the real position!). The function
					// is mostly used to determine whether text contains muster. When muster is not found, it returns
					// std::string::npos.
bool str_begins_with_caseinsensitive(const std::string &text, const std::string &muster, bool word_end, bool leerz = true); // When
					// word_end is set, it returns only true when at the end of text[muster.size()] a space
					// or the ending is (or also '-' and '_'when leerz is set).
size_t str_find_without_whites(const std::string &text, const std::string &wildcard); // Ignores tabulators, spaces, line endings and \r in the text and in the wildcard.
std::string str_remove_sidewhites(const std::string &text); // Removes tabulators, spaces, line endings and \r at the first and at the last position.

uint64_t GetTime_Microseconds(); // 100 years need 52 bit
long long int GetTime_Milliseconds(); // 100 years beed 42 bit -> long long int required (-> 64 bit)

template<typename Tstr> // implemented for std::string and std::basic_string<uint8_t>
std::string FormatToHex(Tstr string, bool small_caps, bool space); // "abc" -> "61 62 63" if space=true

} // namespace stocks_dl

#endif // STOCKS_DL_UTILS_HPP_INCLUDED
