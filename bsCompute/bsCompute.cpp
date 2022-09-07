#include "userinput.h"
#include <cstring>
#include <iostream>
#include <string>

using ::std::cout, ::std::endl, ::std::vector, ::std::string;

int main(int argC, char **argV) {

  user_input input;

  if (argC == 2 && std::strcmp(argV[1], "-version")) {
    ::check_sizes();
    return 0;
  }

  if (!process_user_input(argC, argV, &input)) {
    cout << "Failed to process user input, close." << endl;
    return 1;
  }

  print_user_input(input);

  return 0;
}