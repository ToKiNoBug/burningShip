#include <cstring>
#include <iostream>
#include <string>

#include "userinput.h"

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

  ::bs_center_wind cwind;
  cwind.center = input.center;
  cwind.imag_span = input.startscale;
  cwind.real_span = cwind.imag_span * burning_ship_cols / burning_ship_rows;

  ::mat_age *const mat = new mat_age;

  ::norm2_matc1 *const mat_norm2 =
      (input.mode == compute_mode::with_norm2) ? (new norm2_matc1) : (nullptr);

  ::cplx_matc3 *const mat_cplx_c3 =
      (input.mode == compute_mode::with_cplx_c3) ? (new cplx_matc3) : (nullptr);

  if (mat == nullptr) {
    cout << "Failed to allocate memory for matrix." << endl;
    return 1;
  }

  for (int frameidx = 0; frameidx < input.framecount; frameidx++) {
    std::string filename =
        input.filenameprefix + "frame" + std::to_string(frameidx) + ".bs_frame";

    if (input.compress) {
      filename += ".gz";
    }
    cout << "Computing frame " << frameidx << ", scale = " << cwind.imag_span
         << ", filename = " << filename;

    switch (input.mode) {
    case compute_mode::age_only:
      ::compute_frame_center(mat, cwind, input.maxit);
      break;
    case compute_mode::with_norm2:
      ::compute_frame_norm2c1_center(mat, cwind, input.maxit, mat_norm2);
      break;
    case compute_mode::with_cplx_c3:
      ::compute_frame_cplxmatc3_center(mat, cwind, input.maxit, mat_cplx_c3);
      break;
    }
  }

  delete mat;

  if (mat_norm2 != nullptr) {
    delete mat_norm2;
  }

  if (mat_cplx_c3 != nullptr) {
    delete mat_cplx_c3;
  }

  return 0;
}