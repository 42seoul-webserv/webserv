
#include "ServerManager.hpp"
#include <iostream>
#include <cstring>
#include <sys/stat.h>
int main(int argc, char *argv[])
{
  mkdir("../tempfile", 0777);
  if (THREAD_MODE)
    std::cout << "Thread mode is on, thread number is " << THREAD_NO << '\n';
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
