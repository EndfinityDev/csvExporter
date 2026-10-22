#include "stdafx.h"
#include "HTTPManager.h"


std::string HTTPManager::Get(const std::string& urlHost, const std::string urlPath, const httplib::Params& params, bool allowAnyStatus, int* outStatus)
{
	try
	{
		httplib::Client client(urlHost);
		httplib::Headers headers = {
#ifdef CSVE_HTTP_API_KEY
			{ "X-API-Key", CSVE_HTTP_API_KEY },
#endif
			{ "User-Agent", CSVE_HTTP_USER_AGENT }
		};

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
				// TODO
			}
		}
		else
		{
			// TODO
		}
	}
	catch (const std::exception& e)
	{
		// TODO
	}
	catch (...)
	{
		// TODO
	}
	return "";
}