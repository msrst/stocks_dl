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
/*
 * logger.h
 * default implementation of logger_base
 */

#ifndef dLOGGER_H_INCLUDED
#define dLOGGER_H_INCLUDED

#include <list>
#include <string>
#include <ctime>
#include <fstream>
#include <boost/thread/mutex.hpp>
#include "logger.hpp"

#define LOGGER_MAX_LOGS (100000)

namespace stocks_dl {
namespace logger {

enum logger_meldung_typ
{
    logger_mtyp_fehler_normal = 0,
    logger_mtyp_warnung_normal,
    logger_mtyp_meldung_normal
};
enum logdisplay_typ
{
    logger_ldtyp_textctrl = 0,
    logger_ldtyp_cout,
    logger_ldtyp_datei
};

struct message
{
    logger_meldung_typ typ;
    std::string message_short;
    long long int time;
};
class logdisplay
{
public:
    long int logdisplay_typ;
    virtual ~logdisplay() {}
    virtual void Neue_Zeile(message& meldung) = 0;
    virtual void Init_Aktualisierung() {}
    virtual void End_Aktualisierung() {}
};
class logdisplay_datei: public logdisplay
{
public:
    std::string m_dateiname;
    std::ofstream m_datei;
    bool datei_offen;
    logdisplay_datei();
    virtual ~logdisplay_datei() {}
    virtual void Neue_Zeile(message& meldung);
    virtual void Init_Aktualisierung();
    virtual void End_Aktualisierung();
};

class logger:
    public logger_base
{
private:
    boost::mutex m_meldungen_lock;
    std::list<message> m_meldungen;
    boost::mutex new_messages_lock;
    std::list<message> new_messages;
    std::list<logdisplay*> m_logdisplays;

public:
    logger();
    virtual ~logger();

    virtual void LogError(std::string fehler);
    virtual void LogInformation(std::string meldung);
    virtual void LogWarning(std::string warnung);

    void AddLogDisplay(std::string dateiname);

    void Refresh_LogDisplays(); // Needs to be called cyclicly to write new messages to 
				// the logging file, if registered via AddLogDisplay("filename")

private:
    void RegisterFehler_Warnung_Meldung(message& neue_meldung);
};

} // namespace logger
} // namespace stocks_dl

#endif // dLOGGER_H_INCLUDED
