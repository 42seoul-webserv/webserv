#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <sys/socket.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <ctime>
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

  /* 리소스의 media type 명시 ex. application/json, text/html. */
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
	std::string _version;                            // HTTP/1.1
	int _status_code;                                // 201
	std::string _status_messege;                     // OK
	std::map<std::string, std::string> _description; // other datas...

public: // constructor & destuctor & copy operator
	typedef std::map<std::string, std::string>::const_iterator t_iterator;

	HTTPResponseHeader()
    : _version("HTTP/1.1"), _status_code(-1), _status_messege("null")
  {
    setDefaultHeaderDescription();
  }

	HTTPResponseHeader(const std::string &version, const int &status_code, const std::string &status_messege)
    : _version("HTTP/1.1"), 
      _status_code(-1), 
      _status_messege("null")
  {
    setDefaultHeaderDescription();
  }

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
  }

public: // setter functions
	void addHeader(const std::pair<std::string, std::string>& description_pair)
  {
    this->_description[description_pair.first] = description_pair.second;
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
	std::string getVersion() const
  {
    return this->_version;
  };

	int	        getStatusCode() const
  {
    return this->_status_code;
  };

	std::string getStatusMessege() const
  {
    return this->_status_messege;
  };

  const std::map<std::string, std::string>& getDescription() const
  {
    return this->_description;
  }

  int getContentLength() const
  {
    std::string len = this->_description.find("Content-Length")->second;
    return atoi(len.c_str());
  }

public: // * Interface Functions.
  // join headers to std::string, then return.
  std::string toString() const
  {
    // (1) if _status_code is out of range, throw error
    if (_status_code < 10 || _status_code > 599)
    {
      throw std::runtime_error("Status Code:" + std::to_string(_status_code) + " -> HttpResponse::toString() : status code is out of range\n");
    }

    // (2) if there is no status messege, throw error
    if (_status_messege == "null")
    {
      throw std::runtime_error("HttpResponse::toString() : status messege unset\n");
    }

    // (3) if server name unset.
    if (this->_description.find("Server")->second == "null")
    {
      throw std::runtime_error("HttpResponse::toString() : server name unset.\n");
    }

    // * FIX: std::to_string은 C++11이라 쓰면 안된다!
    std::string header_message = _version + " " + std::to_string(_status_code) + " " + _status_messege + "\r\n";
    HTTPResponseHeader::t_iterator itr = _description.find("Tranfer-Encoding");
    itr = _description.begin();
    bool is_chunked = isTransferChunked();
    while (itr != _description.end())
    {
      if (is_chunked && (itr->first == "Content-Length"))
      {
        // ignore Content-Length
        itr++;
        continue;
      }
      header_message += (itr->first + ": " + itr->second + "\r\n");
      itr++;
    }
    return header_message;
  }

  // check if header has [transfer_encoding : chunked] type
  bool isTransferChunked() const
  {
    bool is_chunked = false;
    HTTPResponseHeader::t_iterator itr = _description.find("Tranfer-Encoding");
    if (itr != _description.end() && itr->second.find("chunked") != std::string::npos)
      is_chunked = true;
    return (is_chunked);
  }


private: // helper functions
	void setDefaultHeaderDescription()
  {
    this->addHeader(HTTPResponseHeader::SERVER("null"));
    this->addHeader(HTTPResponseHeader::DATE());
    this->addHeader(HTTPResponseHeader::CONNECTION("keep-alive"));
    this->addHeader(HTTPResponseHeader::CONTENT_LENGTH(-1));
  } // 헤더 초기값 자동 설정

private: // helper function
};

struct ResponseContext
{
  int fd_file;        // fd for file read
  int fd_socket;      // fd for socket send
  std::string buffer; // string
  struct sockaddr_in addr;
  void (*handler)(struct ResponseContext *obj);
  ServerManager *manager;

public: // contructor
  ResponseContext(int _fd_file, int _fd_socket,
                  std::string _buffer,
                  struct sockaddr_in _addr,
                  void (*_handler)(struct ResponseContext *obj),
                  ServerManager *_manager)
      : fd_file(_fd_file), fd_socket(_fd_socket),
        buffer(_buffer),
        addr(_addr),
        handler(_handler),
        manager(_manager)
  {
  }
};

// ResponseProcessor가 넘겨 받는 데이터.
// 이거의 이름을 HttpResponse로 바꾸기.
class HTTPResponse : public HTTPResponseHeader
{
private:
  FileDescriptor _fd;

public: // constructor & destuctor
  HTTPResponse()
      : HTTPResponseHeader()
  {
  }

  ~HTTPResponse()
  {
  }

public: // setter functions
  FileDescriptor setFd(const FileDescriptor &fd)
  {
    _fd = fd;
  }

public: // getter functions
  HTTPResponseHeader getHeader() const
  {
    // *? 이렇게 다운캐스팅 해서 보내면 될지?
    return static_cast<HTTPResponseHeader>(*this);
  }

  FileDescriptor getFd() const
  {
    return _fd;
  }

public: // interface functions.
  void sendToClient(const HTTPResponse &res, int socket_fd)
  {
  }

private: // Helper function
  // * processResponse를 호출하는 바깥쪽에서 kevent에 등록을 해줘야 할 듯.
  void processResponse(const HTTPResponse &res, struct Context *context)
  {
    // * (1) send header
    {
      //일단 Content Leghth 정보가 들어온다. Content Leght가 0이거나, 상태코드가 300~500 사이면 body는 없다.
      //따라서 fd를 읽지 말고, 그냥 header만 send한다.
      //이때, 404 page 일 경우 body가 있을 수도 있기 때문에, 204만 별도로 처리한다.
      struct kevent event;

      struct ResponseContext *newContext = new struct ResponseContext(context->fd, res.getFd(), res.getHeader().toString(), context->addr, socketSendHandler, context->manager);

      // event 세팅
      EV_SET(&event, newContext->fd_file, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newContext);

      // kevent에 등록한다.
      if (kevent(context->manager->getKqueue(), &event, 0, NULL, 0, NULL) < 0)
      {
        printLog("error: " + getClientIP(&context->addr) + " : event attach failed\n", PRINT_RED);
        throw(std::runtime_error("Event attach failed (response)\n"));
      }
    }

    // * --------------------------------------------------------------------------
    // * | WARN : 이렇게 하면 순서가 꼬여서 헤더보다 바디가 먼저 전송되는거 아니야? 한번 체크해보자.  |
    // * --------------------------------------------------------------------------

    // (2) if body exists, read body and store them to buffer
    //     만약 body가 있다면, (정상코드이고  Content Length가 있다면) 읽는다
    if (res.getFd() != -1 && res.getContentLength() > 0 && res.getStatusCode() != 204)
    {
      // register read handler
      struct kevent event;
      std::string buffer;

      // (2-1) kevent에 read할 fd를 등록 (이제 read_fd가 read 가능해지면 bodyFdReadHandler가 호출된다.

      // ResponseContext 생성.
      struct ResponseContext *newContext = new struct ResponseContext(context->fd, res.getFd(), std::string(""), context->addr, socketSendHandler, context->manager);

      // event 세팅
      EV_SET(&event, newContext->fd_file, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, newContext);

      // kevent에 등록한다.
      if (kevent(context->manager->getKqueue(), &event, 0, NULL, 0, NULL) < 0)
      {
        printLog("error: " + getClientIP(&context->addr) + " : event attach failed\n", PRINT_RED);
        throw(std::runtime_error("Event attach failed (response)\n"));
      }
    }
    else
    // if body doesn't exit, then just close socket.
    {
      //소켓을 완전히 닫고, fd도 닫는다. (connection close?) */
      close(context->fd);
      close(res.getFd());
    }
    delete (context);
  }

  // send body string to socket
  static void socketSendHandler(struct ResponseContext *context)
  {
    // 이 콜백은 socekt send 가능한 시점에서 호출되기 때문에, 이대로만 사용하면 된다.
    if (send(context->fd_socket, context->buffer.c_str(), context->buffer.size(), MSG_DONTWAIT) < 0)
    {
      printLog("error: " + getClientIP(&context->addr) + " : send failed\n", PRINT_RED);
      throw(std::runtime_error("Send Failed\n"));
    }
    delete (context);
  }

  // if fd is not -1, then read data
  static void bodyFdReadHandler(struct ResponseContext *context)
  {
    char buffer[1024] = {0}; // * 버퍼의 크기는 추후에 조정할 것.
    struct kevent event;

    // 만약 데이터가 끝났다면, kevent 종료, fd close
    ssize_t rd_size = read(context->fd_file, buffer, sizeof(buffer));
    if (rd_size < 0)
    {
      // if read failed
      printLog("error: client: " + getClientIP(&context->addr) + " : read failed\n", PRINT_RED);
      throw(std::runtime_error("Read Failed\n"));
    }
    else if (rd_size == 0)
    {
      // if there is nothing to read [ = read done ]
      close(context->fd_file);
    }
    // 데이터가 들어왔다면, 소켓에 현재 버퍼에 있는 데이터를 전송하는 event를 등록.
    else
    {
      // copy buffer to responseContext's buffer
      struct kevent event;

      // ResponseContext를 만들어서 넘긴다.
      struct ResponseContext *newContext = new struct ResponseContext(context->fd_file, context->fd_socket, std::string(buffer), context->addr, socketSendHandler, context->manager);

      // event를 세팅한다.
      EV_SET(&event, newContext->fd_socket, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, newContext);

      // kevent에 등록한다.
      if (kevent(context->manager->getKqueue(), &event, 0, NULL, 0, NULL) < 0)
      {
        printLog("error: " + getClientIP(&context->addr) + " : event attach failed\n", PRINT_RED);
        throw(std::runtime_error("Event attach failed (response)\n"));
      }
    }

    delete (context);
  }

  static std::string getClientIP(const struct sockaddr_in *addr)
  {
    char str[INET_ADDRSTRLEN];
    const struct sockaddr_in *pV4Addr = addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
    return (str);
  }
};

#endif // HTTPResponse.hpp