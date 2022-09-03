#include "burning_ship.h"

#include "renders.h"

#include <stdio.h>

#include <time.h>

#include <omp.h>

#include <stdlib.h>

int main(int argc, char **argv) {

  omp_set_num_threads(4);

  cplx_d Center = -0.591827 - 1.05382i;

  // iterate(C, NULL);

  mat_age *const img = malloc(sizeof(mat_age));

  if (img == NULL) {
    printf("Failed to allocate size for img\n");
    return 1;
  }

  norm2_matc1 *const norm2_mat = malloc(sizeof(norm2_matc1));

  if (norm2_mat == NULL) {
    printf("Failed to allocate size for norm2_mat\n");
    return 1;
  }
  mat_age_f32 *const img_f32 = malloc(sizeof(mat_age_f32));

  if (img_f32 == NULL) {
    printf("Failed to allocate memory for img_f32\n");
    return 1;
  }

  printf("Start computation\n");

  clock_t clk = clock();

  bs_range_wind wind;
  wind.minmin = -2 - 2i;
  wind.maxmax = 2 + 2i;

  const int16_t max_it = 255;

  // compute_frame_range(img, wind, max_it);

  compute_frame_norm2c1_range(img, wind, max_it, norm2_mat);

  clk = clock() - clk;

  printf("Finished computation in %lu miliseconds\n", clk);

  smooth_by_norm2(img, norm2_mat, img_f32);

  write_compressed(img, "data.bs_frame.gz");

  write_abstract_matrix(&norm2_mat->norm2[0][0], sizeof(bs_float),
                        burning_ship_rows, burning_ship_cols,
                        "norm.bs_norm2.gz", true);
  write_abstract_matrix(&img_f32->data[0][0], sizeof(float), burning_ship_rows,
                        burning_ship_cols, "smooth.unknown", false);

  printf("finished writting");

  free(img);
  free(norm2_mat);
  free(img_f32);

  return 0;
}
