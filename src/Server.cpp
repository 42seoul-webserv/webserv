#include "Server.hpp"

static const struct sockaddr_in DEFAULT_ADDR ={
        inet_addr("0.0.0.0"),
        AF_INET,
        htons(42424),
};

MethodType getMethodType(const std::string& method)
{
  const char* methods[] = {"GET", "POST", "PUT", "PATCH", "DELETE"};
  const size_t NUMBER_OF_METHOD = sizeof(methods) / sizeof(char*);

  for (size_t i = 0; i < NUMBER_OF_METHOD; ++i)
  {
    if (method == methods[i])
      return (MethodType(i));
  }
  return (UNDEFINED);
}

Server::Server()
{
}

Server::~Server()
{
  shutdown(_serverFD, SHUT_RDWR);
}