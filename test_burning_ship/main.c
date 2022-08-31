#include "burning_ship.h"

#include <stdio.h>

#include <time.h>

#include <omp.h>

#include <stdlib.h>

int main(int argc, char **argv) {

  omp_set_num_threads(4);

  cplx_d C = 0.375 + 0i;

  // iterate(C, NULL);

  mat_age *img = malloc((burning_ship_rows * burning_ship_cols));

  if (img == NULL) {
    printf("Failed to allocate size");
    return 1;
  }
  printf("Start computation\n");

  clock_t clk = clock();

  compute_frame(img, -2 - 2i, 2 + 2i, 3000);

  clk = clock() - clk;

  printf("Finished computation in %d miliseconds\n", clk);

  write_uncompressed(img, "data");
  // write_compressed(img, "data.gz");

  printf("finished writting");

  free(img);

  return 0;
}
