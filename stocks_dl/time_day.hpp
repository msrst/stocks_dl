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
#ifndef STOCKS_DL_TIME_DAY_HPP_INCLUDED
#define STOCKS_DL_TIME_DAY_HPP_INCLUDED

#include <string>
#include <ctime>

namespace stocks_dl {
	
class time_day
{
private:
    time_t m_time;
    tm m_tmtime;

public:

    time_day();
    time_day(int mday, int mon, int year); // mday: day of month (1 - 31), mon: month (1 - 12), year: the year
private:
    void Init();
public:

    void JumpToNextDay(int days_count = 1); // Jumps days_count days forward. days_count should not be negative.
    time_day As_ToNextDay(int days_count = 1);
    void JumpToNextMonth(int months_count = 1); // Jumps months_count months forward. months_count shoudl not be negative.
    time_day As_ToNextMonth(int monatszahl = 1);
    void JumpToNextTradingDay(); // If he is already a trading day, he does not jump.
    void JumpToLastTradingDay(); // As JumpToNextTradingDay, but backwards.

    void operator= (const time_day& original);
    friend bool operator== (const time_day f1, const time_day f2);
    friend bool operator!= (const time_day f1, const time_day f2);
    friend bool operator< (const time_day f1, const time_day f2);
    friend bool operator<= (const time_day f1, const time_day f2);
    friend bool operator> (const time_day f1, const time_day f2);
    friend bool operator>= (const time_day f1, const time_day f2);
	int operator- (const time_day &subtractor) const {
		return (m_time - subtractor.m_time) / (24*60*60);
	}
	unsigned int TradingDaysSpan(time_day subtractor) const; // only positive spans. He and the subtractor should already be a trading day.

    int GetDayOfMonth() const; // 1 - 31
    int GetMonth() const; // 1 - 12
    int GetYear() const; // p. e. 2017
    int GetYear_Short() const; // p. e. 17 bei 2017

    std::string AsString() const; // nach dem Muster 2017-05-12

    time_t AsTimeT() const;
};


time_day TimeDayFromString(std::string timestring, bool* ok = NULL); // String formed like 2017-05-16 (YYYY-MM-DD)
time_day TimeDayFromDEString(std::string timestring, bool* ok = NULL); // String formed like 16.05.2017 (DD.MM.YYYY)

} // namespace stocks_dl

#endif // STOCKS_DL_TIME_DAY_HPP_INCLUDED
