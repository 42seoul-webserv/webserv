
#include "ServerManager.hpp"
//#include "RequestParser.hpp"
#include <iostream>
#include <cstring>
//int main(int argc, char *argv[], char *envp[])
//{
//  if (argc == 2)
//  {
//    try
//    {
//      ServerManager sv(argv[1]);
//      sv.run();
//    }
//    catch (std::exception& e)
//    {
//      printLog(e.what(), PRINT_RED);
//      return (1);
//    }
//  }
//  else
//  {
//    try
//    {
//      Server newServer(argv[1]);
//      newServer.runServer();
//    }
//    catch (std::exception &e)
//    {
//      std::cout << e.what();
//      return (1);
//    }
//  }
//  return (0);

int main(int ac, char **av)
{
//  int fd;


  ConfigParser parser;
  parser.parseConfigFile(av[1]);
//  std::cout << "\n";
//  parser.displayAll();

  return (0);


//  RequestParser parser;
//  std::string temp;

//  HTTPRequest* request = new HTTPRequest();
//  fd = open(av[1], O_WRONLY);
//  temp += "GET https://www.google.com/search?name=schoe&name1=ming HTTP/1.1\r\n";
//  temp += "Content-Type: text/html;charset=UTF-8\r\n";
//  temp += "Content-Length:3423\r\n";
//  temp += "Content-Encoding: gzip\r\n";
//  temp += "Transfer-Encoding:chunked\r\n\r\n";
//  temp += "5\r\n";
//  temp += "hello\r\n";
//  temp += "5\r\n";
//  temp += "world\r\n";
//  temp += "0\r\n";
//  temp += "\r\n";
//  write(fd, temp.c_str(), temp.size());
//  std::cout << temp.size();
//  close(fd);
//  fd = open(av[1], O_RDONLY);
//  std::cout << fd << std::endl;
//  parser.parseRequest(fd, request);
//  parser.displayAll(request);
}