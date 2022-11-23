#include "RequestParser.hpp"
#include <cstdlib>

std::string::iterator RequestParser::getOneLine(\
    std::string& str, std::string::iterator it, std::string::iterator end)
{
    while (it != end && it + 1 != end)
    {
        if (*it == '\r' && *(it + 1) == '\n')
            return it + 2;
        str += *it;
        it++;
    }
    return it;
}

void RequestParser::chunkedParsing(HTTPRequest* request)
{
    std::string endcheck;
    std::string chunckedbody;
    std::string::reverse_iterator rit;
    std::string::iterator it;
    std::string::iterator end;
    long int length_int;
    char* endptr;
    std::string length_str;
    std::string val;

    for (rit = request->_body.rbegin(); rit != request->_body.rend(); ++rit)
    {
        endcheck.insert(endcheck.begin(), *rit);
        if (endcheck.size() == 5)
            break;
    }
    if (endcheck != "0\r\n\r\n")
        return;
    end = request->_body.end();
    for (it = request->_body.begin(); it != end - 5;)
    {
        it = getOneLine(length_str, it, end);
        it = getOneLine(val, it, end);
        length_int = strtol(length_str.c_str(), &endptr, 10);
        if (*endptr != '\0' || length_int < 0)
            throw std::logic_error("chunked length value error");
        if (val.size() != static_cast<size_t>(length_int))
            throw std::logic_error("chunked length value error");
        chunckedbody += val;
        val.clear();
        length_str.clear();
    }
    request->_body.assign(chunckedbody.begin(), chunckedbody.end());
}

void RequestParser::bodyParsing(HTTPRequest* request)
{
    size_t delim;

    if (!request->_body.size())
    {
        delim = request->_message.find("\r\n\r\n");
        request->_body.assign(request->_message.begin() + delim + 4, request->_message.end());
    }
    if (request->_chunckedFlag)
        chunkedParsing(request);
/*    else
        bodyLengthCheck(request);*/
}

void RequestParser::hearderVaildCheck(HTTPRequest* request)
{
    std::map<std::string, std::string>::iterator length;
    std::map<std::string, std::string>::iterator chunked;
    char *endptr;
    long int length_val;

    length = request->_headers.find("Content-Length");
    chunked = request->_headers.find("Transfer-Encoding");
    if (chunked != request->_headers.end() && chunked->second == "chunked")
        request->_chunckedFlag = true;
    if (!request->_chunckedFlag && length == request->_headers.end())
    {
        throw (std::logic_error("don't have Content-Length"));
    }
    length_val = strtol(length->second.c_str(), &endptr, 10);
    if ((!request->_chunckedFlag && *endptr != '\0') || length_val < 0)
        throw std::logic_error("content-length value error");
    request->_checkLevel = BODY;
    request->_status = HEADEROK;
}

void RequestParser::startLineVaildCheck(HTTPRequest* request)
{
    if (request->_checkLevel != STARTLINE)
        return;
    if (request->_method == UNDEFINED || \
            !request->_url.size() || !request->_version.size())
    {
        throw (std::logic_error("_startline vaild check ERROR"));
    }
    request->_checkLevel = HEADER;
}

void RequestParser::getStartLine(HTTPRequest* request, size_t& end)
{
    std::string::iterator it;
    std::string buffer;

    end = request->_message.find("\r\n");
    if (end == std::string::npos)
    {
        throw (std::logic_error("startline ERROR"));
    }
    it = request->_message.begin();
    for (size_t i = 0, k = 0; i <= end; ++i, ++it)
    {
        if (*it == ' ' || i == end)
        {
            switch (k)
            {
            case 0:
                request->_method = getMethodType(buffer);
                break;
            case 1:
                request->_url = buffer;
                break;
            case 2:
                request->_version = buffer;
                break;
            default:
                throw (std::logic_error("startline ERROR"));
            }
            k++;
            buffer.clear();
        }
        else
            buffer += *it;
    }
}

void RequestParser::getHeader(HTTPRequest* request, size_t begin ,size_t endPOS)
{
  std::string key;
  std::string buffer;
  std::string::iterator it = request->_message.begin();

  for (size_t i = begin; i != endPOS; ++i)
  {
    if (it[i] == '\r' && it[i + 1] == '\n')
    {
      if (!key.empty() && !buffer.empty())
      {
        request->_headers[key] = buffer;
        buffer.clear();
        key.clear();
        i++;
        continue;
      }
      else
      {
        throw (std::logic_error("header key, value error\n"));
      }
    }
    if (it[i] == ':' && key.empty())
    {
      key = buffer;
      buffer.clear();
      if (it[i + 1] == ' ' || it[i + 1] == '\t')
        i++;
    }
    else
    {
      buffer += it[i];
    }
  }
}

//두개 추가 구현 해야함
void RequestParser::eraseFragment(HTTPRequest* request)
{}
void RequestParser::seperateQury(HTTPRequest* request)
{}

void RequestParser::CRLFCheck(HTTPRequest* request)
{
    size_t endPOS = request->_message.find("\r\n\r\n");
    size_t nowPOS;
    
    std::cout << "endPOS : " << endPOS << std::endl;
    if (endPOS != std::string::npos)
    {
        request->_checkLevel = STARTLINE;
        getStartLine(request, nowPOS);
        eraseFragment(request);
        seperateQury(request);
        getHeader(request, nowPOS + 2, endPOS + 2);
    }
}

void RequestParser::RequestParsing(FileDescriptor socektFD, HTTPRequest* request)
{
    char buffer[BUFFER_SIZE] = {0};

    if (request && request->_status != END && request->_status != ERROR)
    {
        if (read(socektFD, buffer, sizeof(buffer)) < 0)
        //if (recv(socektFD, buffer, sizeof(buffer), MSG_DONTWAIT) < 0)???????왜터짐
            throw (std::runtime_error("receive failed\n"));
        try
        {
            if (!request->_body.size())
                request->_message += buffer;
            else
                request->_body += buffer;
            switch (request->_checkLevel)
            {
            case CRLF:
                CRLFCheck(request);
            case STARTLINE:
                startLineVaildCheck(request);
            case HEADER:
                hearderVaildCheck(request);
            case BODY:
                bodyParsing(request);
            }
        }
        catch(const std::exception& Error)
        {
            request->_status = ERROR;
            std::cout << Error.what() << std::endl;
            return ;
        }
    }
}

void RequestParser::displayall(HTTPRequest* request)
{
    std::cout << "method : "<< request->_method << std::endl;
    std::cout << "url : "<< request->_url << std::endl;
    std::cout << "version : "<< request->_version << std::endl;
    for (std::map<std::string, std::string>::iterator it = request->_headers.begin(); \
            it != request->_headers.end(); it++)
    {
        std::cout << it->first << " : " << it->second << std::endl;
    }
    std::cout << "body : "<< request->_body << std::endl;
}