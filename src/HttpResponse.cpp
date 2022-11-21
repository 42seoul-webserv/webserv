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
	return (std::pair<std::string, std::string>("Sever", server_name));
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
// HttpResponseHeader::HttpResponseHeader()
// 		: _version("HTTP/1.1"), _status_code(-1), _status_messege("null")
// {
// }

// HttpResponseHeader::HttpResponseHeader(const std::string &version, const int &status_code,
// 									   const std::string &status_messege)
		// : _version(version), _status_code(status_code), _status_messege(status_messege)
// {
// }

/**----------------------
 * * HttpResponse       |
 *----------------------*/
// * FIXME : context ptr이 free되면 우째 ?

// HttpResponse::HttpResponse(const struct Context *const context_ptr) // defaults to [ HTTP/1.1 | -1 | null ]
		// :HttpResponseHeader(), _context_ptr(context_ptr)
// {
	// this->setDefaultResponseHeader();
// }

// HttpResponse::HttpResponse(const int &status_code, const std::string &status_message, const struct Context *const context_ptr)
		// :HttpResponseHeader(), _context_ptr(context_ptr)
// {
	// this->setDefaultResponseHeader();
	// _status_code = status_code;
	// _status_messege = status_message;
// }

/** 
 * * TODO: 기본 설정중 서버 이름은 server config를 이용해서 적용해야 함. */
// void HttpResponse::setDefaultResponseHeader()
// {
 
// }


void HTTPResponseHeader::setVersion(const std::string &version) // ex. HTTP/1.1
{
	this->_version = version;
}

void HTTPResponseHeader::setStatus(const int &status_code, const std::string &status_messege) // ex. 404 PageNotFound
{
	this->_status_code = status_code;
	this->_status_messege = status_messege;
}













// void HttpResponse::setBody(const std::string &body) {
// 	this->_body = body;
// }

// void HttpResponse::setBody(const char* file_path)
// {
// 	// TODO: 파일을 읽는 이 부분도 kevent를 통해 처리해야 하는지?
// 	std::ifstream file(file_path);
// 	if (file.fail())
// 	{
// 		printLog("error: client: " + getClientIP(&(this->_context_ptr->addr)) +  " : open failed\n", PRINT_RED);
// 		throw std::runtime_error("File Open failed\n");
// 	}
// 	std::string str;
// 	std::string total_read;
// 	while (getline(file, str))
// 	{
// 		_body += (str + "\n");
// 	}
// 	file.close();
// }

// void HttpResponse::setBodyandUpdateContentLength(const char *file_path) {
// 	setBody(file_path);
// 	addHeader(HeaderType::CONTENT_LENGTH(_body.size()));
// }

// void HTTPResponseHeader::addHeader(const std::pair<std::string, std::string> &description_pair) {
// 	this->_description[description_pair.first] = description_pair.second;
// }

// void HTTPResponseHeader::addHeader(const std::string &key, const std::string &value) {
// 	this->_description[key] = value;
// }

// std::string HTTPResponseHeader::getVersion() { return this->_version; }

// int HTTPResponseHeader::getStatusCode() { return this->_status_code; }

// std::string HTTPResponseHeader::getStatusMessege() { return this->_status_messege; }








std::string HTTPResponse::getBody() { return this->_body; }

std::string HTTPResponse::toString() const
{
	// (1) if _status_code is out of range, throw error
	if (_status_code < 10 && _status_code > 599)
	{
		printLog("error: client: " + getClientIP(&(this->_context_ptr->addr)) +  " : Response staus-code out-of-range\n", PRINT_RED);
		throw std::runtime_error("Status Code:" + std::to_string(_status_code) + " -> HttpResponse::toString() : status code is out of range\n");
	}

	// (2) if there is no status messege, throw error
	if (_status_messege == "null")
	{
		printLog("error: client: " + getClientIP(&(this->_context_ptr->addr)) +  " : status-messege unset\n", PRINT_RED);
		throw std::runtime_error("HttpResponse::toString() : status messege is not set\n");
	}

	// (3) if _transfer-encoding-chuinked is set, then ignore Content-Lengths header.
	bool is_chunked = false;
	HTTPResponseHeader::t_iterator itr = _description.find("Tranfer-Encoding");
	if (itr != _description.end() && itr->second.find("chunked") != std::string::npos)
		is_chunked = true;

	std::string message = _version + " " + std::to_string(_status_code) + " " + _status_messege + "\r\n";
	itr = _description.begin();
	while (itr != _description.end())
	{
		if (is_chunked == true && (itr->first == "Content-Length"))
		{
			// ignore Content-Length
			itr++;
			continue;
		}
		message += (itr->first + ": " + itr->second + "\r\n");
		itr++;
	}
	message += ("\n" + _body);
	return (message);
}