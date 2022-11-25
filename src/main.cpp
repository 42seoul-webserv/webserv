
#include "ServerManager.hpp"
#include <iostream>
#include <cstring>
int main(int argc, char *argv[], char *envp[])
{
  if (argc == 2)
  {
    try
    {
      ServerManager sv(argv[1]);
      sv.run();
    }
    catch (std::exception& e)
    {
      printLog(e.what(), PRINT_RED);
      return (1);
    }
  }
  else
  {
    std::cout << "invalid arguments\n";
    return (1);
  }
  return (0);
}