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
#include <curl/curl.h>
#include <cstring>

#include "utils/utils.hpp"
#include "curlwrapper.hpp"

namespace stocks_dl {
	
size_t mcurlwrapper_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	mcurlwrapper *mc = (mcurlwrapper*)userp;
	
	mc->buf.append((char*)contents, realsize);
	return realsize;
}

mcurlwrapper::mcurlwrapper(logger_base_ptr c_logger)
{
	b_initialized = false;
	b_ok = false;
	b_get_configured = true;
	m_b_follow_redirects = false;
	
	m_logger = c_logger;
}
mcurlwrapper::~mcurlwrapper()
{
	if(b_initialized) {
		curl_easy_cleanup(easy_handle);	
	}
}

int mcurlwrapper::Init()
{
	easy_handle = curl_easy_init();
	if(easy_handle) {
		res = curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, mcurlwrapper_write_callback);
		if(res) {
			HandleCurlError("Could not set WriteCallback");
			return 3;
		}
		else {
			res = curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, this);
			if(res) {
				HandleCurlError("Could not set WriteData");
				return 4;
			}
			else {
				// Wichtig ist für boerse-frankfurt.de, dass der user-agent gleich bleibt.
				//res = curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
				res = curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:64.0) Gecko/20100101 Firefox/64.0");
				if(res) {
					HandleCurlError("Could not set user agent");
					return 5;
				}
				else {
					b_ok = true;
					return 0;
				}
			}
		}
		b_initialized = true;
	}
	else {
		m_logger->LogError("Could not create the CURL handle.");
		return 1;
	}
}
int mcurlwrapper::GetRedirect(std::string url, std::string &redirect)
{
	buf.clear();
	res = curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
	if(res) {
		HandleCurlError("Could not set URL");
		return 1;
	}
	if(m_b_follow_redirects) {
		res = curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 0L);
		if(res) {
			HandleCurlError("Could not set redirect to zero.");
			return 2;
		}
		m_b_follow_redirects = false;
	}
	res = curl_easy_perform(easy_handle);
	if(res) {
		HandleCurlError("Could not perform request");
		return 3;
	}
	char *cstr_redirect;
	res = curl_easy_getinfo(easy_handle, CURLINFO_REDIRECT_URL, &cstr_redirect); // man 
					// curl_easy_getinfo says you should not delete variables from
					// curl_easy_getinfo, and in an example from man CURLINFO_REDIRECT_URL
					// it is not deleted.
	if(res) {
		HandleCurlError("Could not get redirect url");
		return 4;
	}
	if(cstr_redirect) {
		redirect = cstr_redirect;
		return 0;
	}
	else {
		long int response_code;
		res = curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
		if(res) {
			HandleCurlError("Could not get HTTP response code (got no redirect URL)");
			return 5;
		}
		else {
			m_logger->LogInformation("Curlwrapper: Got no redirect URL: HTTP response code " + Int_To_String(response_code));
			return 6;
		}
	}
}
int mcurlwrapper::GetSite(std::string url, std::string &sitedata, bool b_follow_redirects)
{
	buf.clear();
	if(!b_get_configured) {
		res = curl_easy_setopt(easy_handle, CURLOPT_HTTPGET, 1L);
		if(res) {
			HandleCurlError("Could not reset to GET");
			return 3;
		}
		b_get_configured = true;
	}
	res = curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
	if(res) {
		HandleCurlError("Could not set URL");
		return 1;
	}
	if(b_follow_redirects != m_b_follow_redirects) {
		if(b_follow_redirects) {
			res = curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
		}
		else {
			res = curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 0L);
		}
		if(res) {
			HandleCurlError("Could not set redirect to \"" + Int_To_String(b_follow_redirects) + "\"");
			return 2;
		}
		m_b_follow_redirects = b_follow_redirects;
	}
	res = curl_easy_perform(easy_handle);
	if(res) {
		HandleCurlError("Could not perform request");
		return 4;
	}
	else {
		sitedata = buf;
		return 0;
	}
}
int mcurlwrapper::GetSitePost(std::string url, const std::string &post_data, std::string &sitedata, bool b_follow_redirects)
{
	buf.clear();
	res = curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
	if(res) {
		HandleCurlError("Could not set URL");
		return 1;
	}
	if(b_follow_redirects != m_b_follow_redirects) {
		if(b_follow_redirects) {
			res = curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
		}
		else {
			res = curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 0L);
		}
		if(res) {
			HandleCurlError("Could not set redirect to \"" + Int_To_String(b_follow_redirects) + "");
			return 2;
		}
		m_b_follow_redirects = b_follow_redirects;
	}
	res = curl_easy_setopt(easy_handle, CURLOPT_COPYPOSTFIELDS, post_data.c_str());
	if(res) {
		HandleCurlError("Could not set post fields");
		return 2;
	}
	b_get_configured = false;
	res = curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, post_data.length());
	if(res) {
		HandleCurlError("Could not set post field size");
		return 3;
	}
	res = curl_easy_perform(easy_handle);
	if(res) {
		HandleCurlError("Could not perform POST request");
		return 4;
	}
	else {
		sitedata = buf;
		return 0;
	}
}
int mcurlwrapper::EnableCookieEngine()
{
	res = curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, ""); // A common trick is to just specify a non-existing file name or plain "" to have it just activate
									// the cookie engine with a blank cookie store to start with.
	if(res) {
		HandleCurlError("Could not activate the cookie engine");
		return 1;
	}
	return 0;
}
int mcurlwrapper::ClearCookies()
{
	res = curl_easy_setopt(easy_handle, CURLOPT_COOKIELIST, "ALL"); // Wipe the entire in-memory cookie storage
	if(res) {
		HandleCurlError("Could not clean the cookie cache");
		return 1;
	}
	return 0;
}
void mcurlwrapper::HandleCurlError(std::string error)
{
	if(error.empty()) {
		m_logger->LogWarning((("Curlwrapper: " + Int_To_String(res) + " ") + curl_easy_strerror(res)) + ".");
	}
	else {
		m_logger->LogWarning((("Curlwrapper: " + error + ": " + Int_To_String(res) + " ") + curl_easy_strerror(res)) + ".");
	}
}

void mcurl_print_slist(curl_slist* slist)
{
	if(slist) {
		std::cout << slist->data << std::endl;
		mcurl_print_slist(slist->next);
	}
}

} // namespace stocks_dl
