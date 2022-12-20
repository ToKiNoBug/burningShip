//
#include <memory.h>
#include <stdio.h>

#include <vector>

#include "high_precision_double.h"

using std::vector;

#include <quadmath.h>

int main() {
  vector<double> fl64;
  vector<hpd> hpd128;
  vector<__float128> fl128;

  const int N = 32;

  __float128 beg = 0.1Q;

  fl64.resize(N);
  hpd128.resize(N);
  fl128.resize(N);

  fl64[0] = beg;
  fl128[0] = beg;
  hpd128[0] = hpd(beg);

  for (int i = 1; i < N; i++) {
    fl64[i] = 4 * fl64[i - 1] * (1 - fl64[i - 1]);
    hpd128[i] = hpd(4) * hpd128[i - 1] * (hpd(1.0) - hpd128[i - 1]);
    fl128[i] = 4 * fl128[i - 1] * (1 - fl128[i - 1]);
  }
  /*
  for (int i = 0; i < N; i++) {
    printf("hpd = %e + %e, fl64 = %e\n", hpd128[i].big, hpd128[i].small,
           fl64[i]);
  }
  return 0;
  */

  constexpr int buffer_size = 4096;
  char buffer[buffer_size] = "";
  /*
    const int temp = quadmath_snprintf(buffer, buffer_size, "%Qf", 1.2Q);

    printf("temp = %i, string = %s\n", temp, buffer);
    */

  printf("The error of double and hpd : \n");

  for (int i = 0; i < N; i++) {
    // memset(buffer, 0, buffer_size);
    const __float128 temp = fl128[i] - __float128(hpd128[i]);
    const int ret =
        quadmath_snprintf(buffer, buffer_size, "%Qe", fl128[i] - fl64[i]);
    if (ret <= 0) {
      printf(
          "\nFunction quadmath_snprintf failed. error code : %i, string = "
          "%s.\n",
          ret, buffer);
      return 1;
    }
    printf("it = %i, %s, %e\n", i, buffer, double(temp));
  }

  printf("\n\n", buffer);

  return 0;
}