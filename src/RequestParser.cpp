#include "RequestParser.hpp"
#include <cstdlib>

std::string::iterator RequestParser::getOneLine(\
    std::string& str, std::string::iterator it, std::string::iterator end)
{
  while (it != end && it + 1 != end)
  {
    if (*it == '\r' && *(it + 1) == '\n')
    {
      return it + 2;
    }
    str += *it;
    it++;
  }
  return it;
}

void RequestParser::parseChunked(HTTPRequest* request)
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

  for (
          rit = request->body.rbegin(); rit != request->body.rend(); ++rit
          )
  {
    endcheck.insert(endcheck.begin(), *rit);
    if (endcheck.size() == 5)
    {
      break;
    }
  }
  if (endcheck != "0\r\n\r\n")
  {
    return;
  }
  end = request->body.end();
  for (
          it = request->body.begin(); it != end - 5;
          )
  {
    it = getOneLine(length_str, it, end);
    it = getOneLine(val, it, end);
    length_int = strtol(length_str.c_str(), &endptr, 10);
    if (*endptr != '\0' || length_int < 0)
    {
      throw std::logic_error("chunked length value error");
    }
    if (val.size() != static_cast<size_t>(length_int))
    {
      throw std::logic_error("chunked length value error");
    }
    chunckedbody += val;
    val.clear();
    length_str.clear();
  }
  request->body.assign(chunckedbody.begin(), chunckedbody.end());
}

void RequestParser::parseBody(HTTPRequest* request)
{
  size_t delim;

  if (!request->body.size())
  {
    delim = request->message.find("\r\n\r\n");
    request->body.assign(request->message.begin() + delim + 4, request->message.end());
  }
  if (request->chunkedFlag)
  {
    parseChunked(request);
  }
/*    else
        checkBodyLength(request);*/
}

void RequestParser::checkHeaderValid(HTTPRequest* request)
{
  std::map<std::string, std::string>::iterator length;
  std::map<std::string, std::string>::iterator chunked;
  char* endptr;
  long int length_val;

  length = request->headers.find("Content-Length");
  chunked = request->headers.find("Transfer-Encoding");
  if (chunked != request->headers.end() && chunked->second == "chunked")
  {
    request->chunkedFlag = true;
  }
  if (!request->chunkedFlag && length == request->headers.end())
  {
    throw (std::logic_error("don't have Content-Length"));
  }
  length_val = strtol(length->second.c_str(), &endptr, 10);
  if ((!request->chunkedFlag && *endptr != '\0') || length_val < 0)
  {
    throw std::logic_error("content-length value error");
  }
  request->checkLevel = BODY;
  request->status = HEADEROK;
}

void RequestParser::checkStartLineValid(HTTPRequest* request)
{
  if (request->checkLevel != STARTLINE)
  {
    return;
  }
  if (request->method == UNDEFINED || \
            !request->url.size() || !request->version.size())
  {
    throw (std::logic_error("_startline vaild check ERROR"));
  }
  request->checkLevel = HEADER;
}

void RequestParser::getStartLine(HTTPRequest* request, size_t& end)
{
  std::string::iterator it;
  std::string buffer;

  end = request->message.find("\r\n");
  if (end == std::string::npos)
  {
    throw (std::logic_error("startline ERROR"));
  }
  it = request->message.begin();
  for (
          size_t i = 0, k = 0; i <= end; ++i, ++it
          )
  {
    if (*it == ' ' || i == end)
    {
      switch (k)
      {
        case 0:
          request->method = getMethodType(buffer);
          break;
        case 1:
          request->url = buffer;
          break;
        case 2:
          request->version = buffer;
          break;
        default:
          throw (std::logic_error("startline ERROR"));
      }
      k++;
      buffer.clear();
    }
    else
    {
      buffer += *it;
    }
  }
}

void RequestParser::getHeader(HTTPRequest* request, size_t begin, size_t endPOS)
{
  std::string key;
  std::string buffer;
  std::string::iterator it = request->message.begin();

  for (
          size_t i = begin; i != endPOS; ++i
          )
  {
    if (it[i] == '\r' && it[i + 1] == '\n')
    {
      if (!key.empty() && !buffer.empty())
      {
        request->headers[key] = buffer;
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
      {
        i++;
      }
    }
    else
    {
      buffer += it[i];
    }
  }
}

//두개 추가 구현 해야함
void RequestParser::eraseFragment(HTTPRequest* request)
{
}

void RequestParser::getQuery(HTTPRequest* request)
{
}

void RequestParser::checkCRLF(HTTPRequest* request)
{
  size_t endPOS = request->message.find("\r\n\r\n");
  size_t nowPOS;

  std::cout << "endPOS : " << endPOS << std::endl;
  if (endPOS != std::string::npos)
  {
    request->checkLevel = STARTLINE;
    getStartLine(request, nowPOS);
    eraseFragment(request);
    getQuery(request);
    getHeader(request, nowPOS + 2, endPOS + 2);
  }
}

void RequestParser::parseRequest(FileDescriptor socketFD, HTTPRequest* request)
{
  char buffer[BUFFER_SIZE] = {0};

  if (request && request->status != END && request->status != ERROR)
  {
    if (read(socketFD, buffer, sizeof(buffer)) < 0)
    {
      //if (recv(socketFD, buffer, sizeof(buffer), MSG_DONTWAIT) < 0)???????왜터짐
      throw (std::runtime_error("receive failed\n"));
    }
    try
    {
      if (!request->body.size())
      {
        request->message += buffer;
      }
      else
      {
        request->body += buffer;
      }
      switch (request->checkLevel)
      {
        case CRLF:
          checkCRLF(request);
        case STARTLINE:
          checkStartLineValid(request);
        case HEADER:
          checkHeaderValid(request);
        case BODY:
          parseBody(request);
      }
    }
    catch (const std::exception& Error)
    {
      request->status = ERROR;
      std::cout << Error.what() << std::endl;
      return;
    }
  }
}

void RequestParser::displayAll(HTTPRequest* request)
{
  std::cout << "method : " << request->method << std::endl;
  std::cout << "url : " << request->url << std::endl;
  std::cout << "version : " << request->version << std::endl;
  for (
          std::map<std::string, std::string>::iterator it = request->headers.begin(); \
            it != request->headers.end(); it++
          )
  {
    std::cout << it->first << " : " << it->second << std::endl;
  }
  std::cout << "body : " << request->body << std::endl;
}