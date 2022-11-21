#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <assert.h>

#include "WebservDefines.hpp"
#include "ServerManager.hpp" // for struct Context define

/**
 * *--------------------------------------------------------------*
 * * [ HeaderType ]                                               |
 * ? * 아래 Subclass는 HttpResponse에 헤더를 추가하기 위한 함수들입니다.     |
 * 사용법 :                                                         |
 *  HttpResponse res;                                             |
 *  res.addHeader(DATE()); --> 헤더 엔티티에 현재 날짜 추가              |
 *  res.addHeader(SERVER("Apache")); --> 헤더 엔티티에 서버 이름 추가    |
 **---------------------------------------------------------------*/
class HeaderType {
public:
  typedef std::pair<std::string, std::string> t_pair;

 /*----------------------------------------------
  * General Header(공통 헤더)
  * 요청과 응답 모두에 적용되지만 바디에서 최종적으로 전송되는 데이터와는 관련이 없는 헤더
  *----------------------------------------------*/

  /* 오늘 날짜를 계산해서 반환함 */
  static t_pair DATE(); // [ Date: ...today's date... ]

  /* ex) Transfer-Encoding: gzip, chunked
  만약 이 부분이 chunked라고 되어 있다면, content-length는 필요 없다. */
  static t_pair TRANSFER_ENCODING(const std::string& transfer_encoding_type);


 /*----------------------------------------------
  * Response Header(응답 헤더)
  * 위치 또는 서버 자체에 대한 정보(이름, 버전)과 같이 응답에 대한 부가적인 정보를 갖는 헤더
  *----------------------------------------------*/

  /* 서버 이름 설정 */
  static t_pair SERVER(const std::string& server_name); // [ Server: server_name ]

  /* 서버측에서 클라이언트에게 세션 쿠키 정보를 설정 (RFC 2965에서 규정) */
  static t_pair SET_COOKIE(const std::string& entity);



 /*----------------------------------------------
  * Entity Header(엔티티 헤더)
  * 컨텐츠 길이나 MIME 타입과 같이 엔티티 바디에 대한 자세한 정보를 포함하는 헤더
  *----------------------------------------------*/

  /* 바이트 단위를 가지는 개체 본문의 크기 */
  static t_pair CONTENT_LENGTH(const size_t& len);

  /* 본문을 이해하는데 가장 적절한 언어 */
  static t_pair CONTENT_LANGUAGE(const std::string& lan);

  /* 리소스의 media type 명시 ex) application/json, text/html. */
  static t_pair CONTENT_TYPE(const std::string& type);

  /* HTTP/1.1 어플리케이션은 트랜잭션이 끝난 다음 커넥션을 끊으려면 Connection:close 헤더를 명시해야 한다.
    ex) Connection: Keep-Alive */
  static t_pair CONNECTION(const std::string& connection_mode);

  /* Allow 헤더는 Access-Control-Allow-Methods랑 비슷하지만, CORS 요청 외에도 적용된다는 데에 차이가 있습니다. 
   즉 GET www.zerocho.com은 되고, POST www.zerocho.com은 허용하지 않는 경우, 
   405 Method Not Allowed 에러를 응답하면서 헤더로 Allow: GET 를 같이 보내면 됩니다. 
   이는 GET 요청만 받겠다는 뜻입니다. 
   * @WARN: Allow: [GET, POST, HEAD] 와 같이 ','와 공백을 넣어야 합니다. */
  static t_pair ALLOW(const std::string& allowed_method);

  /* 300번대 응답이나 201 Created 응답일 때 어느 페이지로 이동할지를 알려주는 헤더입니다.
	HTTP/1.1 302 Found
	Location: /
	이런 응답이 왔다면 브라우저는 / 주소로 리다이렉트합니다. */
  static t_pair LOCATION(const std::string& redirect_location);


 


private: // Helper functions
	static std::string GET_DAY(long tm_wday);
	static std::string GET_MON(long tm_wmon);
	static std::string getDate();
};

/**----------------------
 * * HTTPResponseHeader |
 *----------------------*/
class HTTPResponseHeader : public HeaderType {

protected:
	// 내부에서 예외처리를 하기 위한 Context pointer.
	const struct Context* _context_ptr;

protected:
	std::string _version;                            // HTTP/1.1
	int _status_code;                                // 201
	std::string _status_messege;                     // OK
	std::map<std::string, std::string> _description; // other datas...

public: // constructor & destuctor & copy operator
	typedef std::map<std::string, std::string>::const_iterator t_iterator;

	HTTPResponseHeader(const struct Context * const context_ptr)
    : _context_ptr(context_ptr), _version("HTTP/1.1"), _status_code(-1), _status_messege("null")
  {}

	HTTPResponseHeader(const std::string &version, const int &status_code, const std::string &status_messege, const struct Context * const context_ptr)
    : _version("HTTP/1.1"), 
      _status_code(-1), 
      _status_messege("null"), 
      _context_ptr(context_ptr)
  {}

  HTTPResponseHeader(const HTTPResponseHeader& header)
  {
    *this = header;
  }

  HTTPResponseHeader& operator= (const HTTPResponseHeader& header)
  {
    _version = header._version;
    _status_code = header._status_code;
    _status_messege = header._status_messege;
    _description = header._description;
    _context_ptr = header._context_ptr;
  }

public: // setter functions
	void addHeader(const std::pair<std::string, std::string>& description_pair)
  {
    this->_description[description_pair.first] = description_pair.second;;
  } // add Header via std::pair type argument

	void addHeader(const std::string& key, const std::string& value)
  {
    this->_description[key] = value;
  } // simplest version of addHeader

	void setVersion(const std::string& version) { this->_version = version; };

	void setStatus(const int &status_code, const std::string &status_messege)
  {
    	this->_status_code = status_code;
      this->_status_messege = status_messege;
  };

public: // getter functions
	std::string getVersion()
  {
    return this->_version;
  };

	int	        getStatusCode()
  {
    return this->_status_code;
  };

	std::string getStatusMessege()
  {
    return this->_status_messege;
  };

private: // helper functions
	void setDefaultResponseHeader()
  {
    const in_port_t port_num = this->_context_ptr->addr.sin_port;
    this->addHeader(HTTPResponseHeader::SERVER(this->_context_ptr->manager->getServerName(port_num)));
    this->addHeader(HTTPResponseHeader::DATE());
    this->addHeader(HTTPResponseHeader::CONNECTION("keep-alive"));
  } // 헤더 초기값 자동 설정
};



// ResponseProcessor가 넘겨 받는 데이터.
// 이거의 이름을 HttpResponse로 바꾸기.
class HTTPResponse : public HTTPResponseHeader {
private:
  FileDescriptor  _fd;

public: // constructor & destuctor
  HTTPResponse()
  {}

  ~HTTPResponse()
  {}

public: // setter functions
  FileDescriptor setFd(const FileDescriptor& fd)
  {
    _fd = fd;
  }

public: // getter functions
  HTTPResponseHeader getHeader()
  {
    // *? 이렇게 다운캐스팅 해서 보내면 될지?
    return static_cast<HTTPResponseHeader>(*this);
  }

  FileDescriptor getFd()
  {
    return _fd;
  }


private: // helper functions

};


/*
    HttpResponse response;
    .. after this code, response data is set.
    * 이대, 해당 reponse에 대한 context는 포인터로 저장되어 있음.
    ResponseProcessor(response).process() 하면 끝.

*/

// ResponseProcessor가 변환하는 데이터
// TODO:  이걸  ResponseProcessor에서 그대로 상속받으면 됨. 이 헤더파일에서는 지울 것.
// Response Processor에서는 HttpResponse를 받아서
class ResponseProcessor {

private:
  HTTPResponseHeader  _header;  // Response header.
	std::string	        _body;    // Response body.

public: // Constructor & Destructor

  // 여기에는 res가 들어온다. header와 fd만 존재한다.
	ResponseProcessor(const HTTPResponse& res)
  {};

	~ResponseProcessor()
  {};

public: // Setters
	void setBody(const std::string& body);
	void setBody(const char* file_path);
	// set Body with given file located in [file_path]
	// NOTE: this sets file's total length to header.
	void setBodyandUpdateContentLength(const char* file_path);

public: // Getters
	std::string getBody();


public: // Interface Functions
    // * Process response using given _fd.
    void process(const HTTPResponse& res);



private: // Callback for kevent
    void bodyFdReadHander(); // if fd is not -1, then read data
    void socketSendHandler(); // send string to socket

private: // Inner Interface functions

    // sendHeader()
    // sendBody()

    // * join header and body string, change to full string data
	  std::string	exportToString() const
    {
      // (1) if _status_code is out of range, throw error
      if (_header.getStatusCode() < 10 && _header.getStatusCode() > 599)
      {
        printLog("error: client: " + getClientIP(&(this->_context_ptr->addr)) + " : Response staus-code out-of-range\n", PRINT_RED);
        throw std::runtime_error("Status Code:" + std::to_string(_status_code) + " -> HttpResponse::toString() : status code is out of range\n");
      }

      // (2) if there is no status messege, throw error
      if (_status_messege == "null")
      {
        printLog("error: client: " + getClientIP(&(this->_context_ptr->addr)) + " : status-messege unset\n", PRINT_RED);
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

};


#endif