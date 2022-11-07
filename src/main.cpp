#include <iostream>

int main(int argc, char *argv[], char *envp)
{
  if (argc != 2)
  {
    std::cerr << "INVALID ARGUMENTS";
    return (1);
  }
  return (0);
}