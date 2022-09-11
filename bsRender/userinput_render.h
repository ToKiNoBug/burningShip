#ifndef BSRENDER_USERINPUT_RENDER_H
#define BSRENDER_USERINPUT_RENDER_H

#include <burning_ship.h>

#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

const ::std::unordered_set<::std::string>
    keywords({"-j", "-fps", "-computejson", "-rendermethod", "-pngprefix",
              "-version", "-lightness", "-pngrows", "-pngcols"});

enum class render_method {
  age_linear, // map iteration times to [0,1] linearly
  norm2_only, // map norm2 to [0,1]
  age_q,      // map iteration times  to [0,1] by q-method
  age_norm2_q // map norm2 to [0,1] and then map (age+1-norm2_in_[0,1]) to [0,1]
              // by q-method
};

struct binary_files {
  binary_files() {
    bs_frame = "";
    bs_norm2 = "";
    bs_cplx_c3 = "";
  }
  ::std::string bs_frame;
  ::std::string bs_norm2;
  ::std::string bs_cplx_c3;
};

struct render_options {
public:
  int fps{60};
  render_method method{render_method::age_linear};
  ::std::vector<binary_files> sources;
  ::std::string dest_prefix{""};
  double lightness{0.125};
  int age_maxit;
  int render_maxit;

  int threadnum{(int)::std::thread::hardware_concurrency()};

  int png_rows;
  int png_cols;

  bool is_norm2_ok{false};
  bool is_cplxc3_ok{false};

  inline int png_height() const { return this->png_rows; }
  inline int png_width() const { return this->png_cols; }

  inline int framecount() const { return sources.size(); }
  inline int png_count() const { return fps * framecount(); }
};

void print_help();

bool process_input(const int argCount, const char *const *const argVal,
                   render_options *const dest);

bool check_file_readablity(render_options &);

#endif // BSRENDER_USERINPUT_RENDER_H