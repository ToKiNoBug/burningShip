#include "burning_ship.h"

#include <renders.h>

#include <stdio.h>

#include <time.h>

#include <omp.h>

#include <stdlib.h>

#include <string.h>

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

  const int channels = 3;

  uint8_t *const pixel_data = malloc(burning_ship_rows * burning_ship_cols *
                                     sizeof(uint8_t) * channels);

  if (pixel_data == NULL) {
    printf("Failed to allocate memeory for pixel data\n");
    return 1;
  }

  bs_range_wind wind;
  wind.minmin = -2 - 2i;
  wind.maxmax = 2 + 2i;

  {
    bs_center_wind cwind;
    cwind.center = 0;
    cwind.imag_span = 4;
    cwind.real_span = cwind.imag_span * burning_ship_cols / burning_ship_rows;

    wind = to_range_wind(cwind);
  }

  int16_t max_it = 16;
  {
    int temp;
    printf("max iterations = ");
    scanf_s("%d", &temp);

    if (temp > 0 && temp < 32767) {
      max_it = temp;
    } else {
      printf("Invalid input. Use default value(%d).\n", max_it);
    }
  }

  printf("Start computation\n");

  clock_t clk = clock();
  // compute_frame_range(img, wind, max_it);
  compute_frame_norm2c1_range(img, wind, max_it, norm2_mat);

  clk = clock() - clk;

  printf("Finished computation in %lu miliseconds\n", clk);

  printf("Start rendering\n");
  smooth_by_norm2(img, norm2_mat, img_f32);

  bool ok = true;
  coloring_by_f32_u8c3(img, img_f32, pixel_data);

  char filename[512] = "";

  sprintf(filename, "fractal_u8c3_maxit=%d.png", max_it);

  printf("Start writting\n");
  ok = write_png_u8c3(pixel_data, burning_ship_rows, burning_ship_cols,
                      filename);

  printf("finished writting");

  free(img);
  free(norm2_mat);
  free(img_f32);
  free(pixel_data);

  return 0;
}
