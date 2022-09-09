#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <string>

#include "burning_ship.h"
#include "renders.h"
#include "userinput.h"

using ::std::cout, ::std::endl, ::std::vector, ::std::string;

int main(int argC, char **argV) {

  user_input input;

  if ((argC <= 1)) {
    print_help();
    return 1;
  }

  if (argC == 2 && !std::strcmp(argV[1], "-version")) {
    ::check_sizes();
    return 0;
  }

  if (!process_user_input(argC, argV, &input)) {
    cout << "Failed to process user input, close." << endl;
    return 1;
  }

  print_user_input(input);

  cout << "Press enter to start computation." << endl;
  getchar();

  omp_set_num_threads(input.threadnum);

  ::bs_center_wind cwind;
  cwind.center = input.center;
  cwind.imag_span = input.startscale;
  cwind.real_span = cwind.imag_span * burning_ship_cols / burning_ship_rows;

  ::mat_age *const mat = new mat_age;

  ::norm2_matc1 *const mat_norm2 =
      (input.mode == compute_mode::with_norm2) ? (new norm2_matc1) : (nullptr);

  ::cplx_matc3 *const mat_cplx_c3 =
      (input.mode == compute_mode::with_cplx_c3) ? (new cplx_matc3) : (nullptr);

  uint8_t *u8c1 = (input.compress)
                      ? (new uint8_t[burning_ship_rows * burning_ship_cols])
                      : (nullptr);

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

    cout << "Computing frame " << frameidx
         << ", scale = " << (double)cwind.imag_span
         << ", filename = " << filename << endl;

    std::clock_t clk = std::clock();

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

    clk = std::clock() - clk;

    cout << "Computation takes " << double(clk) * 1000 / CLOCKS_PER_SEC
         << " ms. Exporting file..." << endl;

    if (input.compress) {
      write_compressed(mat, filename.data());
    } else {
      write_uncompressed(mat, filename.data());
    }

    string filename_norm =
        input.filenameprefix + "frame" + std::to_string(frameidx) + ".bs_norm2";
    string filename_cplx_c3 = input.filenameprefix + "frame" +
                              std::to_string(frameidx) + ".bs_cplx_c3";

    switch (input.mode) {
    case compute_mode::with_norm2:
      if (input.compress)
        filename_norm += ".gz";

      write_abstract_matrix(&mat_norm2->norm2[0][0],
                            sizeof(mat_norm2->norm2[0][0]), burning_ship_rows,
                            burning_ship_cols, filename_norm.data(),
                            input.compress);
      break;
    case compute_mode::with_cplx_c3:
      if (input.compress) {
        filename_cplx_c3 += ".gz";
      }
      write_abstract_matrix(&mat_cplx_c3->c3[0][0][0], 3 * sizeof(bs_cplx),
                            burning_ship_rows, burning_ship_cols,
                            filename_cplx_c3.data(), input.compress);
      break;
    default:
      break;
    }

    if (input.preview) {
      ::render_u8c1(mat, u8c1, input.maxit);

      std::string png_name = input.filenameprefix + "frame" +
                             std::to_string(frameidx) + "-preview.png";

      if (!::write_png_u8c1(u8c1, burning_ship_rows, burning_ship_cols,
                            png_name.data())) {
        cout << "Failed to write png file " << png_name
             << ". Press enter to continue or Ctrl+C to terminate computation."
             << endl;
        getchar();
        continue;
      }
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