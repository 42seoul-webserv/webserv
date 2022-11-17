#include "Server.hpp"

static const struct sockaddr_in DEFAULT_ADDR ={
        inet_addr("0.0.0.0"),
        AF_INET,
        htons(42424),
};

Server::Server()
{
}

Server::~Server()
{
  shutdown(_serverFD, SHUT_RDWR);
}

void Server::openServer()
{
  int opt = 1;

  if ((this->_serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    throw (std::runtime_error("Create Socket failed\n"));
  if (setsockopt(this->_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    throw (std::runtime_error("Socket set option failed\n"));
  if (fcntl(this->_serverFD, F_SETFL, O_NONBLOCK) < 0)
    throw (std::runtime_error("fcntl non-block failed\n"));
  if (bind(this->_serverFD, reinterpret_cast<const sockaddr *>(&this->_socketAddr), sizeof(this->_socketAddr)) < 0)
    throw (std::runtime_error("Bind Socket failed\n"));
  if (listen(this->_serverFD, LISTEN_QUEUE_SIZE) < 0)
    throw (std::runtime_error("Listen Socket failed\n"));
}
