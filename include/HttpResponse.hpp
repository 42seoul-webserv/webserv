#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <assert.h>

/**----------------------
 * * HeaderType         |
 *----------------------*/
class HeaderType {
public:
  typedef std::pair<std::string, std::string> t_pair;

  // 오늘 날짜를 계산해서 반환함.
  static t_pair DATE(); // [ Date: ...today's date... ]

  // 서버 이름 설정
  static t_pair SERVER(const std::string& server_name); // [ Server: server_name ]

  // 컨텐츠 길이 설정
  static t_pair CONTENT_LENGTH(const size_t& len);

  // 컨텐츠 언어 설정
  static t_pair CONTENT_LANGUAGE(const std::string& lan);

  // 컨텐츠 타입 설정.
  static t_pair CONTENT_TYPE(const std::string& type);

  // HTTP/1.1 어플리케이션은 트랜잭션이 끝난 다음 커넥션을 끊으려면 Connection:close 헤더를 명시해야 한다.
  // .. ex) Connection: Keep-Alive
  static t_pair CONNECTION(const std::string& connection_mode);

  // ex) Transfer-Encoding: gzip, chunked
  // 만약 이 부분이 chunked라고 되어 있다면, content-length는 필요 없다.
  static t_pair TRANSFER_ENCODING(const std::string& transfer_encoding_type);



private: // Helper functions
	static std::string GET_DAY(long tm_wday);
	static std::string GET_MON(long tm_wmon);
	static std::string getDate();
};

/**----------------------
 * * HttpResponseHeader |
 *----------------------*/
class HttpResponseHeader : public HeaderType {
protected:
	std::string _version;                            // HTTP/1.1
	int _status_code;                                // 201
	std::string _status_messege;                     // OK
	std::map<std::string, std::string> _description; // other datas...

public:
	typedef std::map<std::string, std::string>::const_iterator t_iterator;
	HttpResponseHeader();
	HttpResponseHeader(const std::string &version, const int &status_code, const std::string &status_messege);
};

/*
 [ HttpResponse TODO ]
 * (1) content-length일 경우 Connection: close를 해준다 (필수는 아님)
 * (2) Transfer-Encoding: chunked 일 경우 content-length 헤더를 없애고 chunked 방식 전송을 진행한다. */

class HttpResponse : public HttpResponseHeader {
private:
	std::string	_body;
public: // Constructor & Destructor
	HttpResponse();
	HttpResponse(const int& status_code, const std::string& status_message);
public: // Setters
	void setVersion(const std::string& version);
	void setStatus(const int &status_code, const std::string &status_messege);
	void setBody(const std::string& body);
	void setBody(const char* file_path);

	// set Body with given file located in [file_path]
	// NOTE: this sets file's total length to header.
	void setBodyandUpdateContentLength(const char* file_path);

    // add Header via std::pair type argument
	void addHeader(const std::pair<std::string, std::string>& description_pair);

	// simplest version of addHeader
	void addHeader(const std::string& key, const std::string& value);

public: // Getters
	std::string getVersion();
	int	getStatusCode();
	std::string getStatusMessege();
	std::string getBody();

public: // Interface Functions
	std::string	toString() const;
};

#endif