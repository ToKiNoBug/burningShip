#include "userinput.h"
#include "burning_ship.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>

using ::std::cout, ::std::endl, ::std::vector, ::std::string;

const std::unordered_set<string> keywords = {
    "-centerhex",      "-startscale", "-zoomspeed", "-framecount", "-maxit",
    "-filenameprefix", "-compress",   "-version",   "-mode",       "-j",
    "-preview"};

void print_help() {
  cout << "User guide :\nbsCompute -keywords [value]\n";

  cout << "\n  -version\n";
  cout << "    Display detailed information of this executable.\n";

  cout << "\n  -centerhex <hex>\n";
  cout << "    Input the center (complex number) in hex.\n";
  cout << "    No default value, you must assign the zooming center.\n";

  cout << "\n  -startscale <double>\n";
  cout << "    Input the scale in height at the beginning. If the range is "
          "from -1-2i to 1+2i, then scale in height is 4.\n";
  cout << "    Default value is 5\n";

  cout << "\n  -zoomspeed <double>\n";
  cout << "    Input the speed of zooming in between two frames. For "
          "example, if zoomspeed is 2 and scale is 1, then the next frame\'s "
          "scale will be 0.5\n. Number greater than 1 stands for zooming in, "
          "otherwise zooming away.Only positive number is allowed.\n";
  cout << "    Default value is 2.\n";

  cout << "\n  -framecount <int>\n";
  cout << "    Input the number of frames to be computed, it should be a "
          "positive integer.\n";
  cout << "    Default value is 3\n";

  cout << "\n  -maxit <int>\n";
  cout << "    Input the max iteration times of each point. It shoule be a "
          "positive integer less than 32767.\n";
  cout << "    Default value is 3000\n";

  cout << "\n  -filenameprefix <string>\n";
  cout
      << "    The prefix of generated files. It could be either empty "
         "string, or strings like \"my-custom-prefix_\" or \"D:/fractals/\" or "
         "even \"D:/folder/lalala\"\n";
  cout << "    Default value is empty string.\n";

  cout << "\n  -compress\n";
  cout << "    Tells bsCompute to generate gzip compressed files. This "
          "keyword doesn\'t require a parameter.\n";

  cout << "\n  -mode [ageonly|norm2|cplxc3]\n";
  cout << "    Extra binaries to generate when computing.\n";
  cout << "    Default value is ageonly.\n";

  cout << "\n  -j <int>\n";
  cout << "    Thread number when computing.\n";
  cout << "    Default value is std::thread::hardware_concurrency(), on this "
          "device it is "
       << ::std::thread::hardware_concurrency() << ".\n";

  cout << "\n  -preview\n";
  cout << "    Export a preview image for each frame.\n";
  cout << "    Default value is false.\n";

  cout << endl << endl;
  // cout << "    \n";
}

inline uint8_t hexchar2u8(const char c) noexcept {
  if (c < '0') {
    exit(1);
  }
  if (c <= '9') {
    return c - '0';
  }
  if (c <= 'Z') {
    return 10 + c - 'A';
  }

  return 10 + c - 'a';
}

int hex_to_bin(const char *src, uint8_t *dest) {
  if (src == nullptr || dest == nullptr) {
    return 0;
  }
  if (std::strlen(src) <= 0)
    return 0;
  if (std::strlen(src) % 2 == 1) {
    return 0;
  }

  if (src[0] == '0' && src[1] == 'x') {
    src += 2;
  }

  int n = 0;
  while (*src != '\0') {
    const uint8_t upper4 = hexchar2u8(src[0]);
    const uint8_t lower4 = hexchar2u8(src[1]);

    *dest = (upper4 << 4) | (lower4);
    src += 2;
    dest++;
    n++;
  }

  return n;
}

inline void bin_to_hex(const uint8_t val, char *const dest) {
  const uint8_t high4 = val >> 4;
  dest[0] = (high4 >= 10) ? ('A' + high4 - 10) : ('0' + high4);
  const uint8_t low4 = val & 0b1111;
  dest[1] = (low4 >= 10) ? ('A' + low4 - 10) : ('0' + low4);
}

inline void check_for_redundunt_input(const string &keyword,
                                      const vector<string> &val) {
  if (val.size() > 1) {
    cout << "You input multiple values for" << keyword
         << ", only the last one "
            "will take effect."
         << endl;
  }
}

bool process_user_input(const int argC, const char *const *const argV,
                        user_input *const dest) {
  if (argV == nullptr) {
    return false;
  }

  if (dest == nullptr) {
    return false;
  }

  if (argC <= 1) {
    print_help();
    return true;
  }

  std::unordered_map<string, vector<string>> parameters;
  const char *prev_key = nullptr;
  for (int idx = 1; idx < argC; idx++) {
    string str(argV[idx]);

    if (str[0] == '-') {
      prev_key = parameters.emplace(str, vector<string>()).first->first.data();
    } else {
      if (prev_key == nullptr) {
        cout << "Invalid input! You didn't assign the type for input parameter "
                "\""
             << str << '\"' << endl;
        return false;
      }
      parameters[prev_key].emplace_back(str);
    }
  }

  for (auto it = parameters.begin(); it != parameters.end();) {
    if (!keywords.contains(it->first)) {
      cout << "Invalid input! Unknown keyword \"" << it->first
           << "\" will be ignored." << endl;
      it = parameters.erase(it);
      continue;
    }
    check_for_redundunt_input(it->first, it->second);
    ++it;
  }

  if (parameters.find("-centerhex") == parameters.end()) {
    cout << "Fatal error : you must assign the value of center explicitly."
         << endl;
    return false;
  }

  for (auto &i : parameters) {
    if (i.first == "-centerhex") {
      if (i.second.size() <= 0) {
        cout << "You didn\'t assign the value of centerhex." << endl;
        return false;
      }

      const char *const hex = i.second.back().data();

      uint8_t buffer[1024];

      const int bytes = hex_to_bin(hex, buffer);

      if (bytes != sizeof(bs_cplx)) {
        cout << "The input centerhex is invalid. It contains " << bytes
             << " bytes but requires " << sizeof(bs_cplx) << " bytes." << endl;

        return false;
      }

      memcpy(&dest->center, buffer, bytes);
      continue;
    }

    if (i.first == "-startscale") {
      if (i.second.size() <= 0) {
        //
        return false;
      }

      const char *const ptr = i.second.back().data();

      const double scale = std::atof(ptr);

      if (!std::isfinite(scale) || (scale <= 0)) {
        cout << "Invalid startscale : " << scale << endl;
        return false;
      }

      dest->startscale = scale;
      continue;
    }

    if (i.first == "-zoomspeed") {
      if (i.second.size() <= 0) {
        //
        return false;
      }
      const double zoomspeed = std::atof(i.second.back().data());

      if (!std::isfinite(zoomspeed) || (zoomspeed <= 0) || (zoomspeed == 1)) {
        cout << "Invalid zoomspeed : " << zoomspeed << endl;
      }

      dest->zoomspeed = zoomspeed;
      continue;
    }

    if (i.first == "-framecount") {
      if (i.second.size() <= 0) {
        //
        return false;
      }
      const int framecount = std::atoi(i.second.back().data());

      if (framecount <= 0) {
        cout << "Invalid framecount : " << framecount << endl;
        return false;
      }

      dest->framecount = framecount;
      continue;
    }

    if (i.first == "-maxit") {
      if (i.second.size() <= 0) {
        //
        return false;
      }
      const int maxit = std::atoi(i.second.back().data());

      if (maxit > 32766 || maxit <= 0) {
        cout << "Invalid maxit : " << maxit << endl;
        return false;
      }

      dest->maxit = maxit;
      continue;
    }

    if (i.first == "-filenameprefix") {
      if (i.second.size() <= 0) {
        //
        return false;
      }

      FILE *file = NULL;
      const std::string test_filename =
          i.second.back() + "frame-0_maxit-1000.bs_frame";
      file=fopen(test_filename.data(), "wb");

      if (file == NULL) {
        cout << "Invalid filenameprefix \"" << i.second.back()
             << "\", failed to create testing file " << test_filename << endl;
        return false;
      } else {
        ::fclose(file);
        ::remove(test_filename.data());
      }

      dest->filenameprefix = i.second.back();
      continue;
    }

    if (i.first == "-compress") {
      dest->compress = true;
      continue;
    }

    if (i.first == "-version") {
      ::check_sizes();
      cout << "frame rows = " << burning_ship_rows
           << ", frame cols = " << burning_ship_cols << endl;
      continue;
    }

    if (i.first == "-mode") {
      if (i.second.size() <= 0) {
        return false;
      }

      const std::string &val = i.second.back();
      if (val == "ageonly") {
        dest->mode = compute_mode::age_only;
        continue;
      }
      if (val == "norm2") {
        dest->mode = compute_mode::with_norm2;
        continue;
      }
      if (val == "cplxc3") {
        dest->mode = compute_mode::with_cplx_c3;
        continue;
      }

      cout << "Unknown value for keyword -mode." << endl;
      return false;
    }

    if (i.first == "-j") {
      if (i.second.size() <= 0) {
        return false;
      }

      const int thN = std::atoi(i.second.back().data());

      if (thN <= 0) {
        cout << "Invalid value for thread number : " << thN << endl;
        return false;
      } else {
        dest->threadnum = thN;
        continue;
      }
    }

    if (i.first == "-preview") {
      dest->preview = true;
    }
  }

  return true;
}

void print_user_input(const user_input &input) {

  cout << "Zooming center = " << (double)::bs_real(input.center);
  if (::bs_imag(input.center) >= 0) {
    cout << "+";
  } else {
  }
  cout << (double)::bs_imag(input.center) << "i" << endl;
  cout << "startscale = " << (double)input.startscale << endl;
  cout << "framecount = " << input.framecount << endl;
  cout << "zoomspeed = " << input.zoomspeed << endl;
  cout << "maxit = " << input.maxit << endl;
  cout << "Thread number = " << input.threadnum << endl;
  cout << "filenameprefix = \"" << input.filenameprefix << '\"' << endl;
  cout << "compress = " << (const char *)(input.compress ? "true" : "false")
       << endl;
  switch (input.mode) {
  case compute_mode::age_only:
    cout << "export matrix of int16_t only." << endl;
    break;
  case compute_mode::with_norm2:
    cout << "export matrix of int16_t and norm2." << endl;
    break;
  case compute_mode::with_cplx_c3:
    cout << "export matrix of int16_t and cplx_c3." << endl;
    break;
  }
  cout << "preview = " << (const char *)(input.preview ? "true" : "false")
       << endl;
}

void user_input_to_json(const user_input &input,
                        boost::json::object *const obj_ptr) {
  if (obj_ptr == nullptr) {
    return;
  }

  boost::json::object &obj = *obj_ptr;

  obj["rows"] = (int)burning_ship_rows;
  obj["cols"] = (int)burning_ship_cols;

  obj["size_of_bs_float"] = (int)sizeof(bs_float);

  {
    char hex[2 + sizeof(bs_cplx) * 2 + 1] = "0x";
    const uint8_t *src = (uint8_t *)&input.center;

    for (int byteid = 0; byteid < sizeof(bs_cplx); byteid++) {
      bin_to_hex(src[byteid], hex + 2 + 2 * byteid);
    }

    cout << "hex = " << hex << endl;
    obj["centerhex"] = hex;
  }

  obj["startscale"] = (double)input.startscale;

  obj["maxit"] = input.maxit;

  obj["framecount"] = input.framecount;

  obj["compress"] = input.compress;

  obj["threadnum"] = input.threadnum;

  obj["zoomspeed"] = input.zoomspeed;

  obj["filenameprefix"] = input.filenameprefix;

  obj["preview"] = input.preview;

  switch (input.mode) {
  case compute_mode::age_only:
    obj["mode"] = "ageonly";
    break;
  case compute_mode::with_norm2:
    obj["mode"] = "norm2";
    break;
  case compute_mode::with_cplx_c3:
    obj["mode"] = "cplxc3";
    break;
  }
}