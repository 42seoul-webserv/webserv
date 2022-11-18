#include <iostream>
#include <string>
#include <unistd.h>

#define BUFFER_SIZE 10240

size_t getWordLength(const std::string& str)
{
  size_t utf8_char_count = 0;
  // not yet
}

// CGI ENVIRON..
//
int main(int argc, char *argv[], char *envp[])
{
  char buffer[BUFFER_SIZE] = {0};
  std::string input;

  (void) argv;
  (void) argc;
  (void) envp;

  int readSize = 0;
  while ((readSize = read(0, buffer, BUFFER_SIZE)) > 0)
  {
    input += buffer;
    memset(buffer, 0, sizeof(buffer));
  }
  if (readSize < 0)
    std::cout << "Read Failed\n";
  else
  {
    std::cout << "--Input--\n";
    std::cout << input << "\n";
    std::cout << "Word Length is : " << getWordLength(input) << "\n";
    std::cout << "Byte Length is : "  << input.length() << "\n";
  }
  return (0);
}