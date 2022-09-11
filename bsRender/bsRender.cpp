#include "userinput_render.h"

#include <burning_ship.h>
#include <iostream>
using std::cout, std::endl;
using std::string;

int main(int argC, char **argV) {

  if (argC <= 1) {
    print_help();
    return 0;
  }

  if (argC <= 2 && (argV[1]) == string("-version")) {
    print_version();
    return 0;
  }

  render_options input;
  if (!process_input(argC, argV, &input)) {
    cout << "Error occurred when parsing input. exit." << endl;
    return 1;
  }

  if (!check_file_readablity(input)) {
    return 1;
  }

  print_user_input(input);

  return 0;
}