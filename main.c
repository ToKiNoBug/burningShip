#include "ship_iteration.h"

#include <stdio.h>

int main(int argc, char **argv) {

  cplx_d C = 0.375 + 0i;

  // iterate(C, NULL);

  mat_age *img = malloc(sizeof(mat_age));

  if (img == NULL) {
    printf("Failed to allocate size");
    return 1;
  }
  printf("Start computation\n");

  compute_frame(img, -2 - 2i, 2 + 2i, 3000);

  printf("Finished computation\n");

  write_uncompressed(img, "data");
  // write_compressed(img, "data.gz");

  printf("finished writting");

  free(img);

  return 0;
}
