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

#include "logger.h"

#include "utils.hpp"

#include <boost/thread/lock_guard.hpp>

namespace stocks_dl {
namespace logger {

logdisplay_datei::logdisplay_datei()
{
    datei_offen = false;
}
void logdisplay_datei::Init_Aktualisierung()
{
    m_datei.open(m_dateiname.c_str(), std::ios_base::out | std::ios_base::app); // Creates the file, if necessary. Appends text at the end
    if(m_datei)
    {
        datei_offen = true;
    }
    else
    {
        std::cout << "Logging error: File \"" << m_dateiname << "\" could not be opened. " << std::endl;
    }
}
void logdisplay_datei::Neue_Zeile(message& meldung)
{
    if(datei_offen)
    {
        switch(meldung.typ)
        {
        case logger_mtyp_fehler_normal:
            m_datei << Format_Defaultdate_stdString(meldung.time) << " [error ]: " << meldung.message_short << std::endl;
            break;
        case logger_mtyp_warnung_normal:
            m_datei << Format_Defaultdate_stdString(meldung.time) << " [warn  ]: " << meldung.message_short << std::endl;
            break;
        case logger_mtyp_meldung_normal:
            m_datei << Format_Defaultdate_stdString(meldung.time) << " [inform]: " << meldung.message_short << std::endl;
            break;
        }
    }
}
void logdisplay_datei::End_Aktualisierung()
{
    if(datei_offen)
    {
        m_datei.close();
        datei_offen = false;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------------

logger::logger()
{
    //
}
logger::~logger()
{
    for(std::list<logdisplay*>::iterator it_logdisplay = m_logdisplays.begin(); it_logdisplay != m_logdisplays.end();)
    {
        if(((*it_logdisplay)->logdisplay_typ != logger_ldtyp_datei) && ((*it_logdisplay)->logdisplay_typ != logger_ldtyp_cout)) {
            delete *it_logdisplay;
            it_logdisplay = m_logdisplays.erase(it_logdisplay);
        }
        else {
            it_logdisplay++;
        }
    }
    Refresh_LogDisplays(); // Nicht ganz in Ordnung, aber um des loggens willen...
    for(std::list<logdisplay*>::iterator it_logdisplay = m_logdisplays.begin(); it_logdisplay != m_logdisplays.end(); it_logdisplay++)
    {
        delete *it_logdisplay;
    }
}

void logger::LogError(std::string fehler)
{
    message neuer_fehler;
    neuer_fehler.time = GetTime_Milliseconds();
    neuer_fehler.typ = logger_mtyp_fehler_normal;
    neuer_fehler.message_short = fehler;
    RegisterFehler_Warnung_Meldung(neuer_fehler);
}
void logger::LogWarning(std::string warnung)
{
    message neue_warnung;
    neue_warnung.time = GetTime_Milliseconds();
    neue_warnung.typ = logger_mtyp_warnung_normal;
    neue_warnung.message_short = warnung;
    RegisterFehler_Warnung_Meldung(neue_warnung);
}
void logger::LogInformation(std::string meldung)
{
    message neue_meldung;
    neue_meldung.time = GetTime_Milliseconds();
    neue_meldung.typ = logger_mtyp_meldung_normal;
    neue_meldung.message_short = meldung;
    RegisterFehler_Warnung_Meldung(neue_meldung);
}
void logger::RegisterFehler_Warnung_Meldung(message& neue_meldung)
{
    {
        boost::lock_guard<boost::mutex> locker(m_meldungen_lock);
        m_meldungen.push_back(neue_meldung);
        if(m_meldungen.size() > LOGGER_MAX_LOGS)
        {
            std::list<message>::iterator it_meldung = m_meldungen.begin();
            it_meldung++;
            m_meldungen.erase(m_meldungen.begin(), it_meldung);
        }
    }
    {
        boost::lock_guard<boost::mutex> locker(new_messages_lock);
        new_messages.push_back(neue_meldung);
        if(new_messages.size() > LOGGER_MAX_LOGS)
        {
            std::list<message>::iterator it_meldung = new_messages.begin();
            it_meldung++;
            new_messages.erase(new_messages.begin(), it_meldung);
        }
    }
    std::cout << Format_Defaultdate_stdString(neue_meldung.time) << (neue_meldung.typ == logger_mtyp_fehler_normal ? " [error ]: "
			: (neue_meldung.typ == logger_mtyp_warnung_normal ? " [warn  ]: " : " [inform]: ")) << neue_meldung.message_short << std::endl;
}

void logger::AddLogDisplay(std::string dateiname)
{
    logdisplay_datei* neues_logdisplay = new logdisplay_datei;
    neues_logdisplay->logdisplay_typ = logger_ldtyp_datei;
    neues_logdisplay->m_dateiname = dateiname;
    m_logdisplays.push_back(neues_logdisplay);
}

void logger::Refresh_LogDisplays()
{
    std::list<message> m_new_messages;
    {
        boost::lock_guard<boost::mutex> locker(new_messages_lock);
        m_new_messages = new_messages;
        new_messages.clear();
    }

	if(!m_new_messages.empty()) { // Only create and write to the logging file when necessary
		for(std::list<logdisplay*>::iterator it_logdisplay = m_logdisplays.begin(); it_logdisplay != m_logdisplays.end(); it_logdisplay++)
		{
			(*it_logdisplay)->Init_Aktualisierung();
			for(std::list<message>::iterator it_meldung = m_new_messages.begin(); it_meldung != m_new_messages.end(); it_meldung++)
			{
				(*it_logdisplay)->Neue_Zeile(*it_meldung);
			}
			(*it_logdisplay)->End_Aktualisierung();
		}
    }
}

} // namespace logger
} // namespace stocks_dl
