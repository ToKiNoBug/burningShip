#include "userinput.h"
#include <iostream>
#include <unordered_map>
#include <vector>
using ::std::cout, ::std::endl, ::std::vector, ::std::string;

const std::unordered_set<string> keywords = {
    "-centerhex", "-startscale",     "-zoomspeed", "-framecount",
    "-maxit",     "-filenameprefix", "-compress",  "-version"};

void print_help() {
  cout << "User guide :\nbsCompute -keywords [value]\n";

  cout << "\n  -version\n";
  cout << "    Display detailed information of this executable.\n";

  cout << "\n  -centerhex [hex]\n";
  cout << "    Input the center (complex number) in hex.\n";
  cout << "    No default value, you must assign the zooming center.\n";

  cout << "\n  -startscale [double]\n";
  cout << "    Input the scale in height at the beginning. If the range is "
          "from -1-2i to 1+2i, then scale in height is 4.\n";
  cout << "    Default value is 5\n";

  cout << "\n  -zoomspeed [double]\n";
  cout << "    Input the speed of zooming in between two frames. For "
          "example, if zoomspeed is 2 and scale is 1, then the next frame\'s "
          "scale will be 0.5\n. Number greater than 1 stands for zooming in, "
          "otherwise zooming away.Only positive number is allowed.\n";
  cout << "    Default value is 2.\n";

  cout << "\n  -framecount [int]\n";
  cout << "    Input the number of frames to be computed, it should be a "
          "positive integer.\n";
  cout << "    Default value is 3\n";

  cout << "\n  -maxit [int]\n";
  cout << "    Input the max iteration times of each point. It shoule be a "
          "positive integer less than 32767.\n";
  cout << "    Default value is 3000\n";

  cout << "\n  -filenameprefix [string]\n";
  cout
      << "    The prefix of generated files. It could be either empty "
         "string, or strings like \"my-custom-prefix_\" or \"D:/fractals/\" or "
         "even \"D:/folder/lalala\"\n";
  cout << "    Default value is empty string.\n";

  cout << "\n  -compress\n";
  cout << "    Tells bsCompute to generated gzip compressed files. This "
          "keyword doesn\'t require a parameter.\n";
  cout << endl << endl;
  // cout << "    \n";
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

  for (auto &i : parameters) {
    if (!keywords.contains(i.first)) {
      cout << "Invalid input! Unknown keyword \"" << i.first << '\"' << endl;
      return false;
    }
  }

  return true;
}