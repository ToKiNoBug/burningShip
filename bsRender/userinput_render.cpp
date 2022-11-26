#include "userinput_render.h"

#include <boost/json.hpp>
#include <burning_ship.h>
#include <filesystem>
#include <iostream>
#include <unordered_map>

#include <cstring>
#include <fstream>
#include <zlib.h>

using std::cout, std::endl;
using std::string, std::vector, std::unordered_map;

void print_help() {
  cout << "\nbsRender user guide : \n";

  cout << "command : bsCompute -keyword parameters\n";

  cout << "\n  -j <uint>\n";
  cout << "    The thread number when rendering. Default value is equal to "
          "std::thread::hardware_concurrency(), on your device it is"
       << std::thread::hardware_concurrency() << '\n';

  cout << "\n  -fps <uint>\n";
  cout << "    The number of images of each frame. Default value is 60.\n";

  cout << "\n  -computejson <string>\n";
  cout << "    The \'compute_options.json\' file generated by bsCompute\n";
  cout << "    Default value is \"compute_options.json\"\n";

  cout << "\n  -rendermethod [age_linear|norm2_only|age_q|age_norm2_q]\n";
  cout
      << "    The method of coloring a fractal. Default value is age_linear.\n";

  cout << "\n  -pngprefix <string>\n";
  cout << "    Prefix of generated png files. Default value is empty string.\n";

  cout << "\n  -version\n";
  cout << "    Display details of this executable.\n";

  cout << "\n  -lightness <double>\n";
  cout << "    Set the lightness of every frame. It must be in range (0,1). "
          "Default value is 0.125.\n";

  cout << "\n  -rendermaxit <uint>\n";
  cout << "    Set the max iteration times when rendering. When using "
          "q-methods, q needs to be computed via iterations, then it\'s "
          "necessary to set the max iteration times. It should be at "
          "least 50.\n";

  cout << "\n  -help\n";
  cout << "    Show this list.\n";

  cout << endl << endl;
}

void print_version() {
  ::check_sizes();

  cout << "burning_ship_rows = " << burning_ship_rows
       << ", burning_ship_cols = " << burning_ship_cols << endl;
}

bool process_input(const int argCount, const char *const *const argVal,
                   render_options *const dest) {
  if (dest == nullptr) {
    return false;
  }
  unordered_map<string, vector<string>> args;

  const char *prev_keyword = nullptr;

  for (int idx = 1; idx < argCount; idx++) {
    if (argVal[idx][0] == '-') {
      prev_keyword = argVal[idx];
      args[prev_keyword];
    } else {
      if (prev_keyword == nullptr) {
        cout << "You didn\'t assign a keyword for parameter \"" << argVal[idx]
             << "\", this parameter will be ignored." << endl;
        continue;
      } else {
        args[prev_keyword].emplace_back(argVal[idx]);
      }
    }
  }

  for (auto it = args.begin(); it != args.end();) {
    if (!keywords.contains(it->first)) {
      cout << "Unknown keyword " << it->first << " will be ignored." << endl;
      it = args.erase(it);
    } else {
      ++it;
    }
  }

  if (!args.contains("-computejson")) {
    args["-computejson"].emplace_back("compute_options.json");
  }

  boost::json::object obj;

  for (auto &i : args) {
    if (i.first == "-computejson") {

      if (i.second.size() <= 0) {
        cout << "No parameter for keyword -computejson" << endl;
        return false;
      }

      const string &jsonfile = i.second.back();

      if (!std::filesystem::exists(jsonfile)) {
        cout << "json file " << jsonfile << " doesn\'t exists." << endl;
        return false;
      }

      if (!std::filesystem::is_regular_file(jsonfile)) {
        cout << "json file " << jsonfile << " is not a regular file." << endl;
        return false;
      }
      const size_t jsonfile_size = std::filesystem::file_size(jsonfile);
      // read file and parse json
      {
        char *const buffer = new char[jsonfile_size + 10];

        if (buffer == nullptr) {
          cout << "Failed to create buffer." << endl;
          return false;
        }

        std::ifstream ifile(jsonfile, std::ios::in | std::ios::binary);

        if (!ifile) {
          cout << "Failed to open json file " << jsonfile << endl;
          return false;
        }

        ifile.read(buffer, jsonfile_size);

        ifile.close();

        obj = boost::json::parse(buffer).as_object();
      }

      if ((!obj.contains("rows")) || (!obj.at("rows").is_number())) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"rows\""
             << endl;
        return false;
      }

      if (obj["rows"].as_int64() != burning_ship_rows) {
        cout << "Rows of matrix in jsonfile mismatch with that in the "
                "executable("
             << burning_ship_rows
             << "), please change macros in the source code and compile again."
             << endl;
        return false;
      }

      if ((!obj.contains("cols")) || (!obj.at("cols").is_number())) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"cols\""
             << endl;
        return false;
      }

      if (obj["cols"].as_int64() != burning_ship_cols) {
        cout << "Cols of matrix in jsonfile mismatch with that in the "
                "executable("
             << burning_ship_cols
             << "), please change macros in the source code and compile again."
             << endl;
        return false;
      }

      if ((!obj.contains("size_of_bs_float")) ||
          (!obj.at("size_of_bs_float").is_number())) {

        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"size_of_bs_float\""
             << endl;
        return false;
      }

      if (obj["size_of_bs_float"].as_int64() != sizeof(bs_float)) {
        cout << "Size of bs_float in jsonfile mismatch with that in the "
                "executable("
             << sizeof(bs_float)
             << "), please change macros in the source code and compile again."
             << endl;
        return false;
      }

      if ((!obj.contains("maxit")) || (!obj.at("maxit").is_number())) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"maxit\""
             << endl;
        return false;
      }

      if (obj["maxit"].as_int64() > 0 && obj["maxit"].as_int64() <= 32766) {
        dest->age_maxit = obj["maxit"].as_int64();
      } else {
        cout << "Invalid value for maxit in jsonfile, it should be positive "
                "and no greater than 32766, but in jsonfile it is "
             << obj["maxit"].as_int64() << endl;
        return false;
      }

      if (!obj.contains("zoomspeed") || !obj.at("zoomspeed").is_double()) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"zoomspeed\""
             << endl;
        return false;
      }

      if (obj["zoomspeed"].as_double() <= 0 ||
          obj["zoomspeed"].as_double() == 1) {
        cout << "Zoomspeed in jsonfile must be positive and not equal to 1, "
                "but the input value is "
             << obj["zoomspeed"].as_double() << endl;
        return false;
      } else {
        dest->zoomspeed = obj["zoomspeed"].as_double();
      }

      if ((!obj.contains("framecount")) ||
          (!obj.at("framecount").is_number())) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"framecount\""
             << endl;
        return false;
      }

      if (!obj.contains("files") || !obj.at("files").is_array()) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"files\""
             << endl;
        return false;
      }

      if (obj["files"].as_array().size() != obj["framecount"].as_int64()) {
        cout << "The size of array \"files\" is "
             << obj["files"].as_array().size()
             << ", which is not euqal to \"framecount\"." << endl;
        return false;
      }
      const int source_num = obj["files"].as_array().size();
      dest->sources.resize(source_num);
      for (int idx = 0; idx < source_num; idx++) {
        const auto &array = obj["files"].as_array();

        if (!array.at(idx).is_array()) {
          cout << "Invalid value in field \"files\" : value at index " << idx
               << " should be an array." << endl;
          return false;
        }

        if (array.at(idx).as_array().size() != 3) {
          cout << "Invalid value in field \"files\" : value at index " << idx
               << " is an array but its size should be 3 (infact it is "
               << array.at(idx).as_array().size() << ")." << endl;
          return false;
        }

        const auto &arr_of_strings = array.at(idx).as_array();

        for (int j = 0; j < 3; j++) {
          if (!arr_of_strings.at(j).is_string()) {
            cout << "Invalid value in field \"files\" : value at index " << idx
                 << " is an array, but is has non-string elements at " << j
                 << endl;
          }
        }

        if (arr_of_strings.at(0).as_string().empty()) {
          cout << "Invalid value in field \"files\" : value at index " << idx
               << " is an array of strings, but the first string is empty."
               << endl;
        }

        dest->sources[idx].bs_frame = arr_of_strings.at(0).as_string();
        dest->sources[idx].bs_norm2 = arr_of_strings.at(1).as_string();
        dest->sources[idx].bs_cplx_c3 = arr_of_strings.at(2).as_string();
      }

      if (!obj.contains("mode") || !obj.at("mode").is_string()) {
        cout << "jsonfile incomplete : no value or invalid value for field "
                "\"mode\""
             << endl;
        return false;
      }

      const string sourcemode = obj["mode"].as_string().data();
      bool check_norm2 = false, check_cplxc3 = false;
      if (sourcemode == "norm2") {
        check_norm2 = true;
      }
      if (sourcemode == "cplxc3") {
        check_cplxc3 = true;
      }

      for (int idx = 0; idx < obj["files"].as_array().size(); idx++) {
        const auto &strarr = obj["files"].as_array()[idx].as_array();

        if (check_norm2 && strarr[1].as_string().empty()) {
          cout << "Error in jsonfile : the second string of the " << idx
               << "th element of \"files\" should refer to a regular file, but "
                  "it is empty string."
               << endl;

          return false;
        }

        if (check_cplxc3 && strarr[2].as_string().empty()) {
          cout << "Error in jsonfile : the third string of the " << idx
               << "th element of \"files\" should refer to a regular file, but "
                  "it is empty string."
               << endl;

          return false;
        }
      }

      continue;
    }

    if (i.first == "-j") {
      if (i.second.size() <= 0) {
        return false;
      }

      const int val = std::atoi(i.second.back().data());

      if (val <= 0) {
        cout << "Invalid thread num : " << val << endl;
        return false;
      }

      dest->threadnum = val;
      continue;
    }

    if (i.first == "-fps") {
      if (i.second.size() <= 0) {
        return false;
      }

      const int fps = std::atoi(i.second.back().data());

      if (fps <= 0) {
        cout << "Invalid fps : " << fps << endl;
        return false;
      }
      dest->fps = fps;
    }

    if (i.first == "-rendermethod") {
      if (i.second.size() <= 0) {
        return false;
      }

      const std::string &methodstr = i.second.back();

      if (methodstr == "age_linear") {
        dest->method = render_method::age_linear;
        continue;
      }
      if (methodstr == "norm2_only") {
        dest->method = render_method::norm2_only;
        continue;
      }
      if (methodstr == "age_q") {
        dest->method = render_method::age_q;
        continue;
      }

      if (methodstr == "age_norm2_q") {
        dest->method = render_method::age_norm2_q;
        continue;
      }

      cout << "Unknown render method \"" << methodstr << "\"." << endl;
      return false;
    }

    if (i.first == "-pngprefix") {
      if (i.second.size() <= 0) {
        return false;
      }

      string prefix = i.second.back();

      string testfilename = prefix + "test__114514_1919810_noconflict.png";

      std::ofstream file(testfilename, std::ios::out | std::ios::binary);

      if (!file) {
        cout << "The destprefix \"" << prefix
             << "\" is invalid, failed to create file under it." << endl;
        return false;
      }

      file.close();

      std::filesystem::remove(testfilename);

      dest->dest_prefix = prefix;
      continue;
    }

    if (i.first == "-version") {
      print_version();
      continue;
    }

    if (i.first == "-lightness") {
      if (i.second.size() <= 0) {
        return false;
      }

      const double val = std::atof(i.second.back().data());

      if (val <= 0 || val >= 1) {
        cout << "Lightness should be in range (0,1), but the given value is "
             << val << endl;
        return false;
      }

      dest->lightness = val;
      continue;
    }

    if (i.first == "-rendermaxit") {
      if (i.second.size() <= 0) {
        return false;
      }

      const int val = std::atoi(i.second.back().data());

      if (val < 50) {
        cout << "Invalid value for keyword rendermax. It should be at least 50"
             << endl;
        return false;
      }
      dest->render_maxit = val;
      continue;
    }

    if (i.first == "-pngrows") {
      if (i.second.size() <= 0) {
        return false;
      }

      const int val = std::atoi(i.second.back().data());

      if (val <= 0 || val > burning_ship_rows) {
        cout << "Invalid png rows : " << val << endl;
        return false;
      }
      // dest->png_rows = val;
      continue;
    }

    if (i.first == "-pngcols") {
      if (i.second.size() <= 0) {
        return false;
      }

      const int val = std::atoi(i.second.back().data());

      if (val <= 0 || val > burning_ship_cols) {
        cout << "Invalid png cols : " << val << endl;
        return false;
      }
      // dest->png_cols = val;
      continue;
    }

    if (i.first == "-help") {
      print_help();
      continue;
    }

    if (i.first == "-adaptivef32") {
      dest->self_adaptive_f32 = true;
      continue;
    }
  }

  return true;
}

void print_user_input(const render_options &input) {
  cout << "The render task is : " << endl;
  cout << "FPS = " << input.fps << endl;
  cout << "rendermethod = ";
  switch (input.method) {
  case render_method::age_linear:
    cout << "age_linear";
    break;
  case render_method::age_norm2_q:
    cout << "age_norm2_q";
    break;
  case render_method::norm2_only:
    cout << "norm2_only";
    break;
  case render_method::age_q:
    cout << "age_q";
    break;
  }
  cout << endl;

  cout << "lightness = " << input.lightness << endl;
  cout << "zoomspeed = " << input.zoomspeed << endl;

  cout << "age_maxit = " << input.age_maxit << endl;

  cout << "render_maxit = " << input.render_maxit << endl;

  cout << "self_adaptive_f32 = "
       << (const char *)(input.self_adaptive_f32 ? ("true") : ("false"))
       << endl;

  cout << "threadnum = " << input.threadnum << endl;
  cout << "dest_prefix = \"" << input.dest_prefix << '\"' << endl;
  cout << "size of source files = " << input.sources.size() << endl;
}

bool is_file_readable(const char *const filename) {
  std::ifstream ifile(filename, std::ios::binary);

  if (!ifile) {
    return false;
  }
  ifile.close();
  return true;
}

bool check_sizes_file(const uint8_t __size_of_bs_float,
                      const uint64_t rows_and_cols[2],
                      const char *const filename) {

  if (__size_of_bs_float != size_of_bs_float) {
    cout << "sizeof(bs_float) in file " << filename << " is "
         << __size_of_bs_float << ", but that in this executable is "
         << size_of_bs_float << endl;
    return false;
  }

  if (rows_and_cols[0] != burning_ship_rows) {
    cout << "burning_ship_rows in file " << filename << " is "
         << rows_and_cols[0] << ", but that in this executable is "
         << burning_ship_rows << endl;
    return false;
  }

  if (rows_and_cols[1] != burning_ship_rows) {
    cout << "burning_ship_rows in file " << filename << " is "
         << rows_and_cols[1] << ", but that in this executable is "
         << burning_ship_rows << endl;
    return false;
  }
  return true;
}

bool check_size_nocompress(const char *const filename) {
  FILE *file = NULL;

  file=fopen(filename, "rb");

  if (file == NULL) {
    cout << "Failed to open file " << filename << endl;
    return false;
  }

  uint8_t __size_of_bs_float;
  uint64_t rows_and_cols[2];

  if (1 != ::fread(&__size_of_bs_float, sizeof(uint8_t), 1, file)) {
    cout << "Failed to read sizeof(bs_float) from file " << filename << endl;
    return false;
  }

  if (16 != ::fread(rows_and_cols, 1, 2 * sizeof(uint64_t), file)) {
    cout << "Failed to read rows and cols from file " << filename << endl;
    return false;
  }

  fclose(file);

  return check_sizes_file(__size_of_bs_float, rows_and_cols, filename);
}

bool check_size_compress(const char *const filename) {
  gzFile_s *const zfile = gzopen(filename, "rb");

  if (zfile == NULL) {
    cout << "Failed to open file " << filename << endl;
    return false;
  }

  uint8_t __size_of_bs_float;
  uint64_t rows_and_cols[2];

  if (1 != ::gzfread(&__size_of_bs_float, sizeof(uint8_t), 1, zfile)) {
    cout << "Failed to read file " << filename << endl;
    return false;
  }

  if (16 != ::gzfread(rows_and_cols, 1, 2 * sizeof(uint64_t), zfile)) {
    cout << "Failed to read file " << filename << endl;
    return false;
  }

  gzclose(zfile);

  return check_sizes_file(__size_of_bs_float, rows_and_cols, filename);
}

bool check_file(const char *const filename) {
  std::filesystem::path path(filename);

  if (path.extension() == ".gz") {
    return check_size_compress(filename);
  } else {
    return check_size_nocompress(filename);
  }
}

bool check_file_readablity(render_options &input) {
  const bool check_norm2 = (input.method == render_method::age_norm2_q ||
                            input.method == render_method::norm2_only);
  const bool check_cplxc3 = false; // current value now

  if (input.framecount() <= 0) {
    return false;
  }

  for (int idx = 0; idx < input.framecount(); idx++) {
    if (!check_file(input.sources[idx].bs_frame.data())) {
      return false;
    }

    if (check_norm2 && !check_file(input.sources[idx].bs_norm2.data())) {
      return false;
    }

    if (check_cplxc3 && !check_file(input.sources[idx].bs_cplx_c3.data())) {
      return false;
    }
  }

  return true;
}