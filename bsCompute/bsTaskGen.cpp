#include <omp.h>

#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <thread>

#include "userinput.h"

using ::std::cout, ::std::endl, ::std::string;

int generate_compute_command(const user_input& input, const int argC,
                             const char* const* const argV) {
  boost::json::object jo;
  user_input_to_json(input, &jo);

  {
    std::string command(argV[0]);

    for (int idx = 1; idx < argC; idx++) {
      command += ' ';
      command += argV[idx];
    }
    jo["compute_command"] = command;
  }

  jo["files"] = {boost::json::array()};
  jo["files"].as_array().resize(input.framecount);

  for (auto& arr : (jo["files"].as_array())) {
    arr = {"", "", ""};
  }

  const int frameidx_digits = std::ceil(std::log10(input.framecount) + 1e-3);

  for (int frameidx = 0; frameidx < input.framecount; frameidx++) {
    std::string idx_str = std::to_string(frameidx);
    while (idx_str.size() < frameidx_digits) {
      idx_str.insert(0, 1, '0');
    }
    std::string filename =
        input.filenameprefix + "frame" + idx_str + ".bs_frame";
    if (input.compress) {
      filename += ".gz";
    }
    jo["files"].as_array()[frameidx].as_array()[0] = filename;

    string filename_norm =
        input.filenameprefix + "frame" + idx_str + ".bs_norm2";
    if (input.compress) filename_norm += ".gz";

    string filename_cplx_c3 =
        input.filenameprefix + "frame" + idx_str + ".bs_cplx_c3";
    if (input.compress) {
      filename_cplx_c3 += ".gz";
    }

    switch (input.mode) {
      case compute_mode::with_norm2:
        jo["files"].as_array()[frameidx].as_array()[1] = filename_norm;
        break;

      case compute_mode::with_cplx_c3:
        jo["files"].as_array()[frameidx].as_array()[2] = filename_cplx_c3;
        break;
      default:
        break;
    }
  }

  std::ofstream ofile("compute_options.json", std::ios::out);

  if (!ofile) {
    cout << "Failed to create file "
         << "\"compute_options.json\""
         << ", the json will be exported to console.\n\n\n\n"
         << endl;

    cout << jo << endl;
    cout << "\n\n\n" << endl;

    return 1;
  } else {
    ofile << jo;

    ofile.close();
  }
  cout << "compute_options.json generated." << endl;

  return 0;
}

int main(int argC, char** argV) {
  user_input input;
  // process user input
  {
    if (argC <= 1) {
      print_help();
      return 1;
    }

    if (argC == 2 && !std::strcmp(argV[1], "-version")) {
      ::check_sizes();
      cout << "frame rows = " << burning_ship_rows
           << ", frame cols = " << burning_ship_cols << endl;
      return 0;
    }

    if (!process_user_input(argC, argV, &input)) {
      cout << "Failed to process user input, close." << endl;
      return 1;
    }

    print_user_input(input);
  }

  return generate_compute_command(input, argC, argV);
}