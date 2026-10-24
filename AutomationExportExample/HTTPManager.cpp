#include "stdafx.h"
#include "HTTPManager.h"
#include "StringConverter.h"

std::string HTTPManager::Get(const std::string& urlHost, const std::string urlPath, const httplib::Params& params, bool includeAPIKey, bool allowAnyStatus, int* outStatus)
{
	try
	{
		httplib::Client client(urlHost);
		httplib::Headers headers = {
			{ "User-Agent", CSVE_HTTP_USER_AGENT },
			{ "Connection", "close" }
		};

#ifdef CSVE_HTTP_API_KEY
		if (includeAPIKey)
		{
			headers.emplace("X-API-Key", CSVE_HTTP_API_KEY);
		}
#endif

		std::string queryString = httplib::append_query_params(urlPath, params);

		client.set_connection_timeout(5, 0);
		client.set_read_timeout(5, 0);

		if (auto result = client.Get(queryString, headers))
		{
			if (outStatus)
			{
				*outStatus = result->status;
			}

			if (allowAnyStatus || result->status == 200)
			{
				return result->body;
			}
			else
			{
				Logger::GetLogger()->Log(L"[HTTPManager] Request failed: " + StringConverter::to_wstring(urlHost + urlPath));
				Logger::GetLogger()->Log(L"Status: " + std::to_wstring(result->status));
				Logger::GetLogger()->Log(StringConverter::to_wstring(result->body));
			}
		}
		else
		{
			Logger::GetLogger()->Log(L"[HTTPManager] Request connection failed: " + StringConverter::to_wstring(urlHost + urlPath));
		}
	}
	catch (const std::exception& e)
	{
		Logger::GetLogger()->Log(L"[HTTPManager] Request failed: " + StringConverter::to_wstring(urlHost + urlPath));
		Logger::GetLogger()->Log(StringConverter::to_wstring(e.what()));
	}
	catch (...)
	{
		Logger::GetLogger()->Log(L"[HTTPManager] Request failed: " + StringConverter::to_wstring(urlHost + urlPath));
		Logger::GetLogger()->Log(L"Unknown error");
	}
	return "";
}