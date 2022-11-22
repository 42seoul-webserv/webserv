#include "HttpResponse.hpp"
#include "WebservDefines.hpp"
#include "ServerManager.hpp" // for struct Context define

/**----------------------
 * * HeaderType         |
 *----------------------*/
std::string HeaderType::GET_DAY(long tm_wday) {
	switch (tm_wday)
	{
		case 0: return ("Sun"); break;
		case 1: return ("Mon"); break;
		case 2: return ("Tue"); break;
		case 3: return ("Wed"); break;
		case 4: return ("Thu"); break;
		case 5: return ("Fri"); break;
		case 6: return ("Sat"); break;
		default: return ("null"); break;
	}
}

std::string HeaderType::GET_MON(long tm_wmon) {
	switch (tm_wmon)
	{
		case 0: return ("Jan"); break;
		case 1: return ("Feb"); break;
		case 2: return ("Mar"); break;
		case 3: return ("Apr"); break;
		case 4: return ("May"); break;
		case 5: return ("Jun"); break;
		case 6: return ("Jul"); break;
		case 7: return ("Aug"); break;
		case 8: return ("Sep"); break;
		case 9: return ("Oct"); break;
		case 10: return ("Nov"); break;
		case 11: return ("Dec"); break;
		default: return ("null"); break;
	}
}

std::string HeaderType::getDate() {
	time_t curTime = time(NULL); // get current time info
	struct tm *pLocal = gmtime(&curTime); // convert to struct for easy use
	if (pLocal == NULL)
	{
		// ...
		return ("null");
	}
	std::string result;
	result = GET_DAY(pLocal->tm_wday) + ", " \
					+ std::to_string(pLocal->tm_mday) + " " \
					+ GET_MON(pLocal->tm_mon) + " " \
					+ std::to_string(pLocal->tm_year + 1900) + " GMT";
	return (result);
}

HeaderType::t_pair HeaderType::CONTENT_LENGTH(const size_t &len) {
	return (std::pair<std::string, std::string>("Content-Length", std::to_string(len)));
}

HeaderType::t_pair HeaderType::CONTENT_LANGUAGE(const std::string &lan) {
	return (std::pair<std::string, std::string>("Content-Language", lan));
}

HeaderType::t_pair HeaderType::CONTENT_TYPE(const std::string &type) {
	return (std::pair<std::string, std::string>("Content-type", type));
}

HeaderType::t_pair HeaderType::DATE() {
	return (std::pair<std::string, std::string>("Date", getDate()));
}

HeaderType::t_pair HeaderType::SERVER(const std::string &server_name) {
	return (std::pair<std::string, std::string>("Server", server_name));
}

HeaderType::t_pair HeaderType::CONNECTION(const std::string &connection_mode) {
  return (std::pair<std::string, std::string>("Connection", connection_mode));
}

HeaderType::t_pair HeaderType::TRANSFER_ENCODING(const std::string &transfer_encoding_type) {
  return (std::pair<std::string, std::string>("Transfer-Encoding", transfer_encoding_type));
}

HeaderType::t_pair HeaderType::ALLOW(const std::string &allowd_method) {
  return (std::pair<std::string, std::string>("Allow", allowd_method));
}

HeaderType::t_pair HeaderType::LOCATION(const std::string &redirect_location) {
  return (std::pair<std::string, std::string>("Location", redirect_location));
}

HeaderType::t_pair HeaderType::SET_COOKIE(const std::string &entity) {
  return (std::pair<std::string, std::string>("Set-Cookie", entity));
}

/**----------------------
 * * HttpResponseHeader |
 *----------------------*/


/**----------------------
 * * HttpResponse       |
 *----------------------*/
