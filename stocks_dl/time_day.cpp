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

#include <iostream>

#include "time_day.hpp"
#include "utils/utils.hpp"
#include "decl.hpp"

namespace stocks_dl {
	
time_day::time_day()
{
    Init();

    m_tmtime.tm_mday = 1;
    m_tmtime.tm_mon = 0;
    m_tmtime.tm_year = 0;
    m_time = mktime(&m_tmtime);
}
time_day::time_day(int mday, int mon, int year)
{
    Init();

    m_tmtime.tm_mday = mday;
    m_tmtime.tm_mon = mon - 1;
    m_tmtime.tm_year = year - 1900;
    m_time = mktime(&m_tmtime);
}
void time_day::Init()
{
    m_tmtime.tm_hour = 0;
    m_tmtime.tm_min = 0;
    m_tmtime.tm_sec = 0;
    m_tmtime.tm_isdst = 0; // Is Daylight Saving Time
}

void time_day::JumpToNextDay(int days_count)
{
    m_time += days_count * 24*60*60;
    m_tmtime = *localtime(&m_time);
}

time_day time_day::As_ToNextDay(int days_count)
{
	time_day ret = *this;
	ret.JumpToNextDay(days_count);
	return ret;
}
void time_day::JumpToNextMonth(int months_count)
{
	if(!months_count) {
		return;
	}
	m_tmtime.tm_year += months_count / 12;
	months_count -= months_count / 12;
	m_tmtime.tm_mon += months_count;
	if(m_tmtime.tm_mon + months_count > 11) {
		m_tmtime.tm_mon -= 12;
		m_tmtime.tm_year++;
	}
    m_time = mktime(&m_tmtime);
}
time_day time_day::As_ToNextMonth(int months_count)
{
	time_day ret = *this;
	ret.JumpToNextMonth(months_count);
	return ret;
}
void time_day::JumpToNextTradingDay()
{
	char forwards;
	do {
		if(m_tmtime.tm_wday == 0) { // Sunday
			forwards = 1;
		}
		else if(m_tmtime.tm_wday == 6) { // Saturday
			forwards = 2;
		}
		else if((m_tmtime.tm_mday == 1) && (m_tmtime.tm_mon == 0)) { // New Year's Day (Neujahr)
			forwards = 1;
		}
		else if( (m_tmtime.tm_mon == 11) && ((m_tmtime.tm_mday == 24) || (m_tmtime.tm_mday == 25) || (m_tmtime.tm_mday == 26)
				|| (m_tmtime.tm_mday == 31)) ) { // Christmas Evening, first and second day after Christmas Evening, New Year's Eve (Silvester)
			forwards = 1;
		}
		else {
			forwards = 0;
		}
		JumpToNextDay(forwards);
	} while (forwards);
}
void time_day::JumpToLastTradingDay()
{
	char backwards;
	do {
		if(m_tmtime.tm_wday == 0) { // Sunday
			backwards = 2;
		}
		else if(m_tmtime.tm_wday == 6) { // Saturday
			backwards = 1;
		}
		else if((m_tmtime.tm_mday == 1) && (m_tmtime.tm_mon == 0)) { // New Year's Day (Neujahr)
			backwards = 1;
		}
		else if( (m_tmtime.tm_mon == 11) && ((m_tmtime.tm_mday == 24) || (m_tmtime.tm_mday == 25) || (m_tmtime.tm_mday == 26)
				|| (m_tmtime.tm_mday == 31)) ) { // Christmas Evening, first and second day after Christmas Evening, New Year's Eve (Silvester)
			backwards = 1;
		}
		else {
			backwards = 0;
		}
		JumpToNextDay(-backwards);
	} while (backwards);
}
unsigned int time_day::TradingDaysSpan(time_day subtrahend) const
{
	if(*this > subtrahend) {
		unsigned int zaehler = 0;
		while(subtrahend < *this) {
			subtrahend.JumpToNextDay();
			subtrahend.JumpToNextTradingDay();
			zaehler++;
		}
		return zaehler;
	}
	else {
		return 0;
	}
}
void time_day::operator= (const time_day& original)
{
    m_time = original.m_time;
    m_tmtime = original.m_tmtime;
}
bool operator== (const time_day f1, const time_day f2)
{
    return ((f1.m_tmtime.tm_mday == f2.m_tmtime.tm_mday) && (f1.m_tmtime.tm_mon == f2.m_tmtime.tm_mon) && (f1.m_tmtime.tm_year == f2.m_tmtime.tm_year));
}
bool operator!= (const time_day f1, const time_day f2)
{
    return !(f1 == f2);
}
bool operator< (const time_day f1, const time_day f2)
{
    if(f1.m_tmtime.tm_year < f2.m_tmtime.tm_year) {
        return true;
    }
    else if(f1.m_tmtime.tm_year == f2.m_tmtime.tm_year) {
        if(f1.m_tmtime.tm_mon < f2.m_tmtime.tm_mon) {
            return true;
        }
        else if(f1.m_tmtime.tm_mon == f2.m_tmtime.tm_mon) {
            return (f1.m_tmtime.tm_mday < f2.m_tmtime.tm_mday);
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}
bool operator<= (const time_day f1, const time_day f2)
{
    if(f1.m_tmtime.tm_year < f2.m_tmtime.tm_year) {
        return true;
    }
    else if(f1.m_tmtime.tm_year == f2.m_tmtime.tm_year) {
        if(f1.m_tmtime.tm_mon < f2.m_tmtime.tm_mon) {
            return true;
        }
        else if(f1.m_tmtime.tm_mon == f2.m_tmtime.tm_mon) {
            return (f1.m_tmtime.tm_mday <= f2.m_tmtime.tm_mday);
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}
bool operator> (const time_day f1, const time_day f2)
{
    return f2 < f1;
}
bool operator>= (const time_day f1, const time_day f2)
{
    return f2 <= f1;
}

int time_day::GetDayOfMonth() const
{
    return m_tmtime.tm_mday;
}
int time_day::GetMonth() const
{
    return m_tmtime.tm_mon + 1;
}
int time_day::GetYear() const
{
    return m_tmtime.tm_year + 1900;
}
int time_day::GetYear_Short() const
{
    return m_tmtime.tm_year % 100;
}

std::string time_day::AsString() const
{
    return Int_To_String_Zeros(GetYear(), 4) + '-' + Int_To_String_Zeros(GetMonth(), 2)
        + '-' + Int_To_String_Zeros(GetDayOfMonth(), 2);
}

time_t time_day::AsTimeT() const
{
    return m_time;
}

//-----------------------------------------------------------------------------------------------------------------------

time_day TimeDayFromString(std::string timestring, bool* ok)
{
    if(timestring.length() == 10) {
        if((timestring[4] == '-') && (timestring[7] == '-')) {
            string_to_int_limited_converter convertobj;
            int mday, mon, year;
            std::string::iterator it_zeichen = timestring.begin();
            std::string::iterator it_zeichen2 = it_zeichen + 4;
            year = convertobj.ConvertToLLIFromSIt_Limited(it_zeichen, it_zeichen2,
                    DATE_MIN_YEAR, DATE_MAX_YEAR);
            it_zeichen2++;
            it_zeichen = it_zeichen2 + 2;
            mon = convertobj.ConvertToLLIFromSIt_Limited(it_zeichen2, it_zeichen, 1, 12);
            it_zeichen++;
            mday = convertobj.ConvertToLLIFromSIt_Limited(it_zeichen, timestring.end(), 1, 31);

            if((!convertobj.MistakeHappened()) && (!convertobj.LimitMistakeHappened())) {
                if(ok != NULL) {
                    *ok = true;
                }
                return time_day(mday, mon, year);
            }
        }
    }
    if(ok != NULL) {
        *ok = false;
    }
    return time_day();
}
time_day TimeDayFromDEString(std::string timestring, bool* ok)
{
    if(timestring.length() == 10) {
        if((timestring[2] == '.') && (timestring[5] == '.')) {
            string_to_int_limited_converter convertobj;
            int mday, mon, year;
            std::string::iterator it_zeichen = timestring.begin();
            std::string::iterator it_zeichen2 = it_zeichen + 2;
            mday = convertobj.ConvertToLLIFromSIt_Limited(it_zeichen, it_zeichen2, 1, 31);
            it_zeichen2++;
            it_zeichen = it_zeichen2 + 2;
            mon = convertobj.ConvertToLLIFromSIt_Limited(it_zeichen2, it_zeichen, 1, 12);
            it_zeichen++;
            year = convertobj.ConvertToLLIFromSIt_Limited(it_zeichen, timestring.end(),
                    DATE_MIN_YEAR, DATE_MAX_YEAR);

            if((!convertobj.MistakeHappened()) && (!convertobj.LimitMistakeHappened())) {
                if(ok != NULL) {
                    *ok = true;
                }
                return time_day(mday, mon, year);
            }
        }
    }
    if(ok != NULL) {
        *ok = false;
    }
    return time_day();
}

} // namespace stocks_dl
