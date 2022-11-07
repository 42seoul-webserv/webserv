#include "webserv.hpp"

int main(int argc, char *argv[], char *envp[])
{
  if (argc != 2)
  {
    std::cerr << "INVALID ARGUMENTS\n";
    return (1);
  }
  else
  {
    parseConfigure(argv[1]);
  }
  return (0);
}