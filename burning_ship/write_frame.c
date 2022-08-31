#include "burning_ship.h"
#include <stdio.h>
#include <string.h>
#include <zlib.h>

bool write_uncompressed(const mat_age *img, const char *path) {

  if (strlen(path) <= 0)
    return false;

  FILE *ofile = fopen(path, "wb");

  if (ofile == NULL) {

    return false;
  }

  fwrite(img, 1, (burning_ship_rows * burning_ship_cols), ofile);

  return true;
}

bool write_compressed(const mat_age *img, const char *path) {
  gzFile zFile = gzopen(path, "wb");

  if (zFile == NULL) {
    return false;
  }

  const size_t rows = burning_ship_rows;
  const size_t cols = burning_ship_cols;

  gzfwrite(&rows, 1, sizeof(size_t), zFile);
  gzfwrite(&cols, 1, sizeof(size_t), zFile);

  gzfwrite(img, 1, (burning_ship_rows * burning_ship_cols), zFile);

  gzclose(zFile);

  return true;
}