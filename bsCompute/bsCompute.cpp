#include <iostream>
#include <string>

#include "userinput.h"

using ::std::cout, ::std::endl, ::std::vector, ::std::string;

int main(int argC, char **argV) {

  user_input input;

  if (!process_user_input(argC, argV, &input)) {
    return 1;
  }

  return 0;
}