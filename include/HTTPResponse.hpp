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
#include "Session.hpp"

struct Context;
/**
 * *--------------------------------------------------------------*
 * * [ HeaderType ]                                               |
 * ? * 아래 Subclass는 HTTPResponse에 헤더를 추가하기 위한 함수들입니다.     |
 * 사용법 :                                                         |
 *  HTTPResponse res;                                             |
 *  res.addHeader(DATE()); --> 헤더 엔티티에 현재 날짜 추가              |
 *  res.addHeader(SERVER("Apache")); --> 헤더 엔티티에 서버 이름 추가    |
 **---------------------------------------------------------------*/
class HeaderType
{
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
    static t_pair SERVER(
            const std::string& serverName); // [ Server: serverName ]

    /* 서버측에서 클라이언트에게 세션 쿠키 정보를 설정 (RFC 2965에서 규정) */
    static t_pair SET_COOKIE(const std::string& entity);

    /*----------------------------------------------
     * Entity Header(엔티티 헤더)
     * 컨텐츠 길이나 MIME 타입과 같이 엔티티 바디에 대한 자세한 정보를 포함하는 헤더
     *----------------------------------------------*/

    /* 바이트 단위를 가지는 개체 본문의 크기 */
    static t_pair CONTENT_LENGTH(const ssize_t& len);

    /* 본문을 이해하는데 가장 적절한 언어 */
    static t_pair CONTENT_LANGUAGE(const std::string& language);

    /* 리소스의 media type 명시 ex. application/json, text/html. */
    static t_pair CONTENT_TYPE(const std::string& type);

    /* HTTP/1.1 어플리케이션은 트랜잭션이 끝난 다음 커넥션을 끊으려면 Connection:close 헤더를 명시해야 한다.
      ex) Connection: Keep-Alive */
    static t_pair CONNECTION(const std::string& connectionMode);

    /* Allow 헤더는 Access-Control-Allow-Methods랑 비슷하지만, CORS 요청 외에도 적용된다는 데에 차이가 있습니다.
     즉 GET www.zerocho.com은 되고, POST www.zerocho.com은 허용하지 않는 경우,
     405 Method Not Allowed 에러를 응답하면서 헤더로 Allow: GET 를 같이 보내면 됩니다.
     이는 GET 요청만 받겠다는 뜻입니다.
     * @WARN: Allow: [GET, POST, HEAD] 와 같이 ','와 공백을 넣어야 합니다. */
    static t_pair ALLOW(const std::string& allowedMethod);

    /* 300번대 응답이나 201 Created 응답일 때 어느 페이지로 이동할지를 알려주는 헤더입니다.
    HTTP/1.1 302 Found
    Location: /
    이런 응답이 왔다면 브라우저는 / 주소로 리다이렉트합니다. */
    static t_pair LOCATION(const std::string& redirect_location);

    static std::string getDateByYearOffset(int year_diff);
    static std::string getDateByHourOffset(int hour_diff);

private: // Helper functions
    static std::string GET_DAY(long tm_wday);
    static std::string GET_MON(long tm_wmon);
    static std::string getDate();
    // 현재 시간 기준 과거 시간 대를 string 으로 반환.
};

/**----------------------
 * * HTTPResponseHeader |
 *----------------------*/
class HTTPResponseHeader : public HeaderType
{

protected:
    std::string _version;                            // HTTP/1.1
    int _status_code;                                // 201
    std::string _statusMessage;                     // OK
    std::map<std::string, std::string> _description; // other datas...

public: // * constructor & destuctor & copy operator
    typedef std::map<std::string, std::string>::const_iterator t_iterator;

    HTTPResponseHeader();
    HTTPResponseHeader(const std::string& version, const int& statusCode,
                       const std::string& statusMessage,
                       const std::string& serverName);
    HTTPResponseHeader(const HTTPResponseHeader& header);
    HTTPResponseHeader& operator=(const HTTPResponseHeader& header);

public:                                                                        // * setter functions
    void addHeader(
            const std::pair<std::string, std::string>& descriptionPair); // add Header via std::pair type argument
    void addHeader(const std::string& key,
                   const std::string& value);            // simplest version of addHeader
    void setVersion(const std::string& version);
    void setStatus(const int& statusCode, const std::string& statusMessage);

public: // * getter functions
    std::string getVersion() const;
    int getStatusCode() const;
    std::string getStatusMessage() const;
    const std::map<std::string, std::string>& getDescription() const;
    ssize_t getContentLength() const;

public:                           // * Interface Functions.
    std::string toString() const;   // join headers to std::string, then return.
    bool isTransferChunked() const; // check if header has [transfer_encoding : chunked] type

private:                              // helper functions
    void setDefaultHeaderDescription(); // 헤더 초기값 자동 설정
};


/**----------------------
 * * HttpResponse       |
 *----------------------*/
class HTTPResponse : public HTTPResponseHeader
{
private:
    FileDescriptor _fileFd;

public: // * constructor & destuctor
    // set statusCode, statusMessage, and serverName.
    HTTPResponse(const int& statusCode, const std::string& statusMessage,
                 const std::string& serverName);
    ~HTTPResponse();

public: // * setter functions
    void setFd(const FileDescriptor& fd);

public: // * getter functions
    HTTPResponseHeader getHeader() const;
    FileDescriptor getFd() const;

public: // * interface functions
    void sendToClient(struct Context* context);

private: // * helper functions
    static void socketSendHandler(struct Context* context);
    static void bodyFdReadHandler(struct Context* context);
    static std::string getClientIP(const struct sockaddr_in* addr);
};

#endif // HTTPResponse.hpp