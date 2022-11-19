#include "HttpResponse.hpp"

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

/**----------------------
 * * HttpResponseHeader |
 *----------------------*/
HttpResponseHeader::HttpResponseHeader()
		: _version("HTTP/1.1"), _status_code(-1), _status_messege("null")
{}

HttpResponseHeader::HttpResponseHeader(const std::string &version, const int &status_code,
									   const std::string &status_messege)
		: _version(version), _status_code(status_code), _status_messege(status_messege)
{}


/**----------------------
 * * HttpResponse       |
 *----------------------*/
HttpResponse::HttpResponse() // defaults to [ HTTP/1.1 | -1 | null ]
		:HttpResponseHeader()
{}

HttpResponse::HttpResponse(const int &status_code, const std::string &status_message)
		:HttpResponseHeader()
{
	_status_code = status_code;
	_status_messege = status_message;
}

void HttpResponse::setVersion(const std::string &version) // ex. HTTP/1.1
{
	this->_version = version;
}

void HttpResponse::setStatus(const int &status_code, const std::string &status_messege) // ex. 404 PageNotFound
{
	this->_status_code = status_code;
	this->_status_messege = status_messege;
}

void HttpResponse::setBody(const std::string &body) {
	this->_body = body;
}

void HttpResponse::setBody(const char* file_path)
{
	std::ifstream file(file_path);
	if (file.fail())
	{
		throw std::runtime_error("File Open failed\n");
	}
	std::string str;
	std::string total_read;
	while (getline(file, str))
	{
		_body += (str + "\n");
	}
}

void HttpResponse::setBodyandUpdateContentLength(const char *file_path) {
	setBody(file_path);
	addHeader(HeaderType::CONTENT_LENGTH(_body.size()));
}

void HttpResponse::addHeader(const std::pair<std::string, std::string> &description_pair) {
	this->_description[description_pair.first] = description_pair.second;
}

void HttpResponse::addHeader(const std::string &key, const std::string &value) {
	this->_description[key] = value;
}

std::string HttpResponse::getVersion() { return this->_version; }

int HttpResponse::getStatusCode() { return this->_status_code; }

std::string HttpResponse::getStatusMessege() { return this->_status_messege; }

std::string HttpResponse::getBody() { return this->_body; }

std::string HttpResponse::toString() const
{
	// (1) if _status_code is out of range, throw error
	if (_status_code < 10 && _status_code > 599)
		throw std::runtime_error("Status Code:" + std::to_string(_status_code) + " -> HttpResponse::toString() : status code is out of range\n");

	// (2) if there is no status messege, throw error
	if (_status_messege == "null")
		throw std::runtime_error("HttpResponse::toString() : status messege is not set\n");

	// (3) if _transfer-encoding-chuinked is set, then ignore Content-Lengths header.
	bool is_chunked = false;
	HttpResponseHeader::t_iterator itr = _description.find("Tranfer-Encoding");
	if (itr != _description.end() && itr->second.find("chunked") != std::string::npos)
		is_chunked = true;

	std::string message = _version + " " + std::to_string(_status_code) + " " + _status_messege + "\r\n";
	itr = _description.begin();
	while (itr != _description.end())
	{
		if (is_chunked == true && (itr->first == "Content-Length"))
		{
			itr++;
			continue;
		}
		message += (itr->first + ": " + itr->second + "\r\n");
		itr++;
	}
	message += ("\n" + _body);
	return (message);
}