#include <omp.h>

#include <atomic>
#include <boost/json.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "burning_ship.h"
#include "renders.h"
#include "userinput.h"

std::atomic<bool> terminate_after_current_frame = false;

using ::std::cout, ::std::endl, ::std::vector, ::std::string;

user_input json_to_user_input(std::string_view jsonfilename,
                              std::vector<uint8_t>* const is_frame_computed);

void receive_input();

void run_tasks(const user_input& input, const int tasks_to_do,
               const std::vector<uint8_t>& is_frame_computed);

int main(int argC, char** argV) {
  const char* task_filename = "./compute_options.json";
  // process cmdline input
  if (argC == 2) {
    task_filename = argV[1];
  }

  if (argC > 2) {
    cout << "Invalid input : more than 2 parameters." << endl;
    return 1;
  }
  std::vector<uint8_t> is_frame_computed;
  const user_input input =
      json_to_user_input(task_filename, &is_frame_computed);
  int tasks_to_do = 0;
  for (uint8_t val : is_frame_computed) {
    if (!val) tasks_to_do++;
  }

  cout << "Task json parsed without error." << endl;

  if (tasks_to_do <= 0) {
    cout << "No work to do. All tasks finished." << endl;
    return 0;
  }
  cout << tasks_to_do << " tasks to compute." << endl;
  //////////////////////////////////////////////////////
  cout << "Computation will start. Input \"pause\" and bsTaskRun will pause "
          "once "
          "current frame is finished. If you want to stop immediately, press "
          "ctrl+C."
       << endl;

  std::thread wait_for_user_input(receive_input);

  run_tasks(input, tasks_to_do, is_frame_computed);

  if (!::terminate_after_current_frame) {
    ::terminate_after_current_frame = true;
    cout << "Computation finished or paused." << endl;
    wait_for_user_input.detach();
    try {
      exit(0);
    } catch (...) {
      return 0;
    }
    // cout << "All tasks finished." << endl;
  } else {
    cout << "Computation finished or paused." << endl;
    wait_for_user_input.join();
    // cout << "Tasks paused successfully." << endl;
  }

  return 0;
}

user_input json_to_user_input(std::string_view jsonfilename,
                              std::vector<uint8_t>* const is_frame_computed) {
  boost::json::object jo;
  // read file
  {
    std::ifstream ifile;

    ifile.open(jsonfilename.data(), std::ios::in | std::ios::binary);

    if (!ifile) {
      cout << "Failed to open file " << jsonfilename << endl;
      exit(1);
      return user_input();
    }

    const size_t jsonfile_size = std::filesystem::file_size(jsonfilename);
    char* const buffer = new char[jsonfile_size + 10];
    ifile.read(buffer, jsonfile_size);

    ifile.close();
    jo = boost::json::parse(buffer).as_object();

    delete[] buffer;
  }

  user_input input;

  if (jo.contains("rows") && jo.at("rows").is_int64()) {
    const int rows_json = jo.at("rows").as_int64();
    if (rows_json != burning_ship_rows) {
      cout << "Invalid value of rows." << endl;
      exit(1);
    }
  } else {
    cout << "No value for rows in json" << endl;
    exit(1);
  }

  if (jo.contains("cols") && jo.at("cols").is_int64()) {
    const int rows_json = jo.at("cols").as_int64();
    if (rows_json != burning_ship_cols) {
      cout << "Invalid value of cols." << endl;
      exit(1);
    }
  } else {
    cout << "No value for cols in json" << endl;
    exit(1);
  }

  if (jo.contains("size_of_bs_float") && jo.at("size_of_bs_float").is_int64()) {
    const int rows_json = jo.at("size_of_bs_float").as_int64();
    if (rows_json != sizeof(bs_float)) {
      cout << "Invalid value of size_of_bs_float." << endl;
      exit(1);
    }
  } else {
    cout << "No value for size_of_bs_float in json" << endl;
    exit(1);
  }

  if (jo.contains("centerhex") && jo.at("centerhex").is_string()) {
    const char* hexstr = jo.at("centerhex").as_string().data();

    uint8_t buffer[1024];

    const int bytes = hex_to_bin(hexstr, buffer);

    if (bytes != sizeof(bs_cplx)) {
      cout << "Invalid value for centerhex in json. It contains " << bytes
           << " bytes but requires " << sizeof(bs_cplx) << " bytes." << endl;
      exit(1);
    }

    memcpy(&input.center, buffer, sizeof(bs_cplx));

  } else {
    cout << "No valid value for centerhex in json." << endl;
    exit(1);
  }

  if (jo.contains("startscale") && jo.at("startscale").is_double()) {
    input.startscale = jo.at("startscale").as_double();
  } else {
    cout << "No valid value for startscale in json" << endl;
    exit(1);
  }

  if (jo.contains("maxit") && jo.at("maxit").is_int64()) {
    if (jo.at("maxit").as_int64() <= 0 || jo.at("maxit").as_int64() >= 32767) {
      cout << "Value of maxit out of range." << endl;
      exit(1);
    }
    input.maxit = jo.at("maxit").as_int64();
  } else {
    cout << "No valid value for maxit in json" << endl;
    exit(1);
  }

  if (jo.contains("framecount") && jo.at("framecount").is_int64()) {
    input.framecount = jo.at("framecount").as_int64();

  } else {
    cout << "No valid value for framecount in json" << endl;
    exit(1);
  }

  if (jo.contains("compress") && jo.at("compress").is_bool()) {
    input.compress = jo.at("compress").as_bool();
  } else {
    cout << "No valid value for compress in json" << endl;
    exit(1);
  }

  if (jo.contains("threadnum") && jo.at("threadnum").is_int64()) {
    input.threadnum = jo.at("threadnum").as_int64();

    if (input.threadnum <= 1) {
      cout << "Invalid value for threadnum in json" << endl;
      exit(1);
    }
  } else {
    cout << "No valid value for threadnum in json" << endl;
    exit(1);
  }

  if (jo.contains("zoomspeed") && jo.at("zoomspeed").is_double()) {
    input.zoomspeed = jo.at("zoomspeed").as_double();

    if (input.zoomspeed <= 0) {
      cout << "Invalid value for zoomspeed in json" << endl;
      exit(1);
    }
  } else {
    cout << "No valid value for zoomspeed in json" << endl;
    exit(1);
  }

  if (jo.contains("preview") && jo.at("preview").is_bool()) {
    input.preview = jo.at("preview").as_bool();
  } else {
    cout << "No valid value for preview in json" << endl;
    exit(1);
  }

  if (jo.contains("filenameprefix") && jo.at("filenameprefix").is_string()) {
    input.filenameprefix = jo.at("filenameprefix").as_string();
  } else {
    cout << "No valid value for filenameprefix in json" << endl;
    exit(1);
  }

  if (jo.contains("mode") && jo.at("mode").is_string()) {
    std::string_view temp = jo.at("mode").as_string();
    bool ok = false;
    //[ageonly|norm2|cplxc3]
    if (temp == "norm2") {
      input.mode = compute_mode::with_norm2;
      ok = true;
    }
    if (temp == "ageonly") {
      input.mode = compute_mode::age_only;
      ok = true;
    }
    if (temp == "cplxc3") {
      input.mode = compute_mode::with_cplx_c3;
      ok = true;
    }

    if (!ok) {
      cout << "Invalid value for mode in json." << endl;
      exit(1);
    }
  } else {
    cout << "No valid value for mode in json" << endl;
    exit(1);
  }

  if (jo.contains("files") && jo.at("files").is_array()) {
    const auto& ja = jo.at("files").as_array();

    if (ja.size() != input.framecount) {
      cout << "Conflicts in json : framecount=" << input.framecount
           << ", while the length of files is " << ja.size() << endl;
      exit(1);
    }

    is_frame_computed->resize(input.framecount);
    memset(is_frame_computed->data(), 0, input.framecount);

    for (int i = 0; i < input.framecount; i++) {
      if (ja.at(i).is_array() && ja.at(i).as_array().size() == 3) {
        const auto& jaja = ja.at(i).as_array();
        bool is_this_frame_finished = true;
        for (int j = 0; j < 3; j++) {
          std::string_view filename = jaja.at(j).as_string();

          if ((filename.size() <= 0) ||
              std::filesystem::is_regular_file(filename)) {
            // if the filename is empty, this file is not requried.
            // if the filename is not empty, then this file should be generated.
          } else {
            is_this_frame_finished = false;
          }
        }

        is_frame_computed->at(i) = is_this_frame_finished;

      } else {
        cout << "Invalid value : the " << i
             << "-th element of files is not an array, or its length is not 3."
             << endl;
        exit(1);
      }
    }

  } else {
    cout << "No value for files in json." << endl;
    exit(1);
  }

  return input;
}

void receive_input() {
  std::string str;
  while (true) {
    if (terminate_after_current_frame) {
      // if the main loop set it to true, all tasks are finished and we don't
      // need to wait for input.
      break;
    }
    std::cin >> str;

    if (str != "pause") {
      cout << "Unknown instruction\"" << str
           << "\" during computation. Wait for "
              "another "
              "input."
           << endl;
    } else {
      cout << "Computation will pause once current frame is finished. For "
              "instant abort, press ctrl+C."
           << endl;
      terminate_after_current_frame = true;
      break;
    }
  }
}

void run_tasks(const user_input& input, const int tasks_to_do,
               const std::vector<uint8_t>& is_frame_computed) {
  const int frameidx_digits = std::ceil(std::log10(input.framecount) + 1e-3);

  int task_counter = 0;

  constexpr int alignment = 32;

  ::mat_age* const age_mat =
      (mat_age*)aligned_alloc(alignment, sizeof(mat_age));
  ::mat_age_f32* const age_mat_f32 =
      (mat_age_f32*)aligned_alloc(alignment, sizeof(mat_age_f32));
  ::norm2_matc1* const norm2_mat =
      (norm2_matc1*)aligned_alloc(alignment, sizeof(norm2_matc1));
  ::cplx_matc3* const cplxc3_mat =
      (cplx_matc3*)aligned_alloc(alignment, sizeof(cplx_matc3));
  uint8_t* const u8c1 = (uint8_t*)aligned_alloc(
      alignment, sizeof(uint8_t) * burning_ship_rows * burning_ship_cols);

  if (age_mat == nullptr || age_mat_f32 == nullptr || norm2_mat == nullptr ||
      cplxc3_mat == nullptr || u8c1 == nullptr) {
    cout << "Failed to allocate memory for matrices. Abort." << endl;
    exit(1);
    return;
  }

  for (int frameidx = 0; frameidx < input.framecount; frameidx++) {
    if (is_frame_computed[frameidx]) {
      continue;
    }

    std::string idx_str = std::to_string(frameidx);
    while (idx_str.size() < frameidx_digits) {
      idx_str.insert(0, 1, '0');
    }
    std::string filename_frame =
        input.filenameprefix + "frame" + idx_str + ".bs_frame";

    if (input.compress) {
      filename_frame += ".gz";
    }
    ::bs_center_wind cwind;

    cwind.center = input.center;
    cwind.imag_span = input.startscale / std::pow(input.zoomspeed, frameidx);
    cwind.real_span = cwind.imag_span * burning_ship_cols / burning_ship_rows;

    cout << "[ " << (task_counter + 1) << " , " << tasks_to_do << " ] : ";
    cout << "Computing frame " << frameidx
         << ", scale = " << (double)cwind.imag_span
         << ", filename = " << filename_frame << endl;

    double clk_seconds = omp_get_wtime();
    // compute frames
    switch (input.mode) {
      case compute_mode::age_only:
        ::compute_frame_center(age_mat, cwind, input.maxit);
        break;
      case compute_mode::with_norm2:
        ::compute_frame_norm2c1_center(age_mat, cwind, input.maxit, norm2_mat);
        break;
      case compute_mode::with_cplx_c3:
        ::compute_frame_cplxmatc3_center(age_mat, cwind, input.maxit,
                                         cplxc3_mat);
        break;
    }

    clk_seconds = omp_get_wtime() - clk_seconds;

    cout << "            "
         << "Computation finished in " << clk_seconds << " seconds." << endl;

    if (::terminate_after_current_frame) {
      cout << "pause signal received. bsTaskRun will terminate once current "
              "frame is exported to disk. patience..."
           << endl;
    }
    clk_seconds = omp_get_wtime();
    bool ok = true;
    // export frame files
    if (input.compress) {
      ok = write_compressed(age_mat, filename_frame.data());
    } else {
      ok = write_uncompressed(age_mat, filename_frame.data());
    }

    if (!ok) {
      cout << "\n\nError : Failed to generate frame file " << filename_frame
           << ", press enter to ignore this error and continue." << endl;
      getchar();
    } else {
      // cout << "Generated " << filename_frame << endl;
    }

    // export norm2 files and cplxc3 files.

    string filename_norm =
        input.filenameprefix + "frame" + idx_str + ".bs_norm2";
    string filename_cplx_c3 =
        input.filenameprefix + "frame" + idx_str + ".bs_cplx_c3";
    switch (input.mode) {
      case compute_mode::with_norm2:
        if (input.compress) filename_norm += ".gz";

        // jo["files"].as_array()[frameidx].as_array()[1] = filename_norm;

        ok = write_abstract_matrix(&norm2_mat->norm2[0][0],
                                   sizeof(norm2_mat->norm2[0][0]),
                                   burning_ship_rows, burning_ship_cols,
                                   filename_norm.data(), input.compress);
        if (!ok) {
          cout << "\n\nError : Failed to generate file " << filename_norm
               << ", press enter to ignore this error and continue." << endl;
          getchar();
        } else {
          // cout << "Generated " << filename_norm << endl;
        }
        break;
      case compute_mode::with_cplx_c3:
        if (input.compress) {
          filename_cplx_c3 += ".gz";
        }
        // jo["files"].as_array()[frameidx].as_array()[2] = filename_cplx_c3;

        ok = write_abstract_matrix(
            &cplxc3_mat->c3[0][0][0], 3 * sizeof(bs_cplx), burning_ship_rows,
            burning_ship_cols, filename_cplx_c3.data(), input.compress);
        if (!ok) {
          cout << "\n\nError : Failed to generate file " << filename_cplx_c3
               << ", press enter to ignore this error and continue." << endl;
          getchar();
        } else {
          // cout << "Generated " << filename_cplx_c3 << endl;
        }
        break;
      default:
        break;
    }

    // generate prewiew images
    if (input.preview) {
      // cout << "rendering preview image..." << endl;
      ::render_u8c1(age_mat, u8c1, input.maxit);

      std::string png_name =
          input.filenameprefix + "frame" + idx_str + "-preview.png";
      // cout << "exporting prewive image..." << endl;
      if (!::write_png_u8c1(u8c1, burning_ship_rows, burning_ship_cols,
                            png_name.data())) {
        cout << "\n\nError : Failed to write png file " << png_name
             << ". Press enter to ignore this error and continue, or press "
                "Ctrl+C to terminate computation."
             << endl;
        getchar();
        continue;
      }
    }

    clk_seconds = omp_get_wtime() - clk_seconds;

    cout << "            "
         << "Exporting finished in " << clk_seconds << " seconds." << endl;
    if (::terminate_after_current_frame) {
      break;
    }

    task_counter++;
  }

  if (!::terminate_after_current_frame) {
    cout << "All tasks finished." << endl;
  } else {
    cout << "Tasks paused successfully." << endl;
  }

  free(age_mat);
  free(age_mat_f32);
  free(norm2_mat);
  free(cplxc3_mat);
  free(u8c1);
}