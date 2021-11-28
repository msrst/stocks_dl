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
#ifndef STOCKS_DL_CURLWRAPPER_HPP_INCLUDED
#define STOCKS_DL_CURLWRAPPER_HPP_INCLUDED

#include <map>
#include <string>
#include <curl/curl.h>
#include "decl.hpp"
#include "utils/logger.hpp"

namespace stocks_dl {

class mcurlwrapper
{
private:
	logger_base_ptr m_logger;
	CURLcode res;
	bool b_initialized;
	bool b_ok;
	std::string buf;
	bool b_get_configured;
	bool m_b_follow_redirects;

public:
	CURL *easy_handle;

	mcurlwrapper(logger_base_ptr c_logger);
	~mcurlwrapper();

	int Init();
	bool IsOk() {
		return b_ok;
	}
	int GetRedirect(std::string url, std::string &redirect);
	int GetSite(std::string url, std::string &sitedata, bool b_follow_redirects = false, std::map<std::string, std::string> headers = std::map<std::string, std::string>());
	int GetSitePost(std::string url, const std::string &post_data, std::string &sitedata, bool b_follow_redirects = false);
	int EnableCookieEngine();
	int ClearCookies();

	friend size_t mcurlwrapper_write_callback(void *contents, size_t size, size_t nmemb, void *userp);

private:
	void HandleCurlError(std::string error = std::string());
};

void mcurl_print_slist(curl_slist* slist);

} // namespace stocks_dl

#endif // STOCKS_DL_CURLWRAPPER_HPP_INCLUDED
