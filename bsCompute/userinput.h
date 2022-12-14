#ifndef BSCOMPUTE_USERINPUT_H
#define BSCOMPUTE_USERINPUT_H

#include <burning_ship.h>

#include <boost/json.hpp>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>


extern const std::unordered_set<::std::string> keywords;

enum class compute_mode {
  age_only,
  with_norm2,
  with_cplx_c3,
};

struct user_input {
  bs_cplx center{0};
  bs_float startscale{4};
  int16_t maxit{3000};
  int framecount{3};
  double zoomspeed{2};
  bool compress{false};
  compute_mode mode{compute_mode::age_only};
  ::std::string filenameprefix{""};
  unsigned int threadnum{::std::thread::hardware_concurrency()};
  bool preview{false};
};

/*
struct user_input_2 : public user_input{
  std::vector<std::string> target_files;
};

*/

void print_help();

bool process_user_input(const int argC, const char *const *const argV,
                        user_input *const dest);

void print_user_input(const user_input &input);

void user_input_to_json(const user_input &input,
                        boost::json::object *const obj);

int hex_to_bin(const char *src, uint8_t *dest);

#endif  // BSCOMPUTE_USERINPUT_H