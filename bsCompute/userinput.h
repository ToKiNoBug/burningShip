#ifndef BSCOMPUTE_USERINPUT_H
#define BSCOMPUTE_USERINPUT_H

#include <burning_ship.h>
#include <string>
#include <unordered_set>
#include <vector>

extern const std::unordered_set<::std::string> keywords;

struct user_input {
  bs_cplx center{0};
  bs_float startscale{4};
  int16_t maxit{3000};
  int framecount{3};
  double zoomspeed{2};
  bool compress{false};
  ::std::string filenameprefix{""};
};

void print_help();

bool process_user_input(const int argC, const char *const *const argV,
                        user_input *const dest);

void print_user_input(const user_input &input);

#endif // BSCOMPUTE_USERINPUT_H