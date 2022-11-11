
#include "ServerManager.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
  if (argc != 2)
  {
    try
    {
      ServerManager sv("");
      sv.run();
    }
    catch (std::exception& e)
    {
      std::cout << e.what();
      return (1);
    }
  }
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
  return (0);
}