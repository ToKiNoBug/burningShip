#include "userinput_render.h"

#include <burning_ship.h>
#include <iostream>

#include <omp.h>

using std::cout, std::endl;
using std::string;

int main(int argC, char **argV) {

  if (argC == 2) {
    if (argV[1] == string("-version")) {
      print_version();
      return 0;
    }
    if (argV[1] == string("-help")) {
      print_help();
      return 0;
    }
  }

  render_options input;
  if (!process_input(argC, argV, &input)) {
    cout << "Error occurred when parsing input. exit." << endl;
    return 1;
  }

  if (!check_file_readablity(input)) {
    return 1;
  }

  omp_set_num_threads(input.threadnum);

  print_user_input(input);

  cout << "Press enter to confirm and start rendering." << endl;

  getchar();

  execute_rendering(input);

  return 0;
}