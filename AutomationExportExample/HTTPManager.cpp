#include "stdafx.h"
#include "HTTPManager.h"


std::string HTTPManager::Get(std::string url)
{
	try
	{
		httplib::Client client(url);

		client.set_connection_timeout(5, 0);
		client.set_read_timeout(5, 0);

		if (auto result = client.Get(url.c_str()))
		{
			if (result->status == 200)
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