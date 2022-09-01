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
  const uint64_t rows = burning_ship_rows;
  const uint64_t cols = burning_ship_cols;

  fwrite(&rows, sizeof(uint64_t), 1, ofile);
  fwrite(&cols, sizeof(uint64_t), 1, ofile);

  fwrite(img, sizeof(int16_t), (burning_ship_rows * burning_ship_cols), ofile);

  fclose(ofile);

  return true;
}

bool write_compressed(const mat_age *img, const char *path) {
  gzFile zFile = gzopen(path, "wb");

  if (zFile == NULL) {
    return false;
  }

  const uint64_t rows = burning_ship_rows;
  const uint64_t cols = burning_ship_cols;

  gzfwrite(&rows, sizeof(uint64_t), 1, zFile);
  gzfwrite(&cols, sizeof(uint64_t), 1, zFile);

  gzfwrite(img, 1, (burning_ship_rows * burning_ship_cols) * sizeof(int16_t),
           zFile);

  gzclose(zFile);

  return true;
}

bool write_abstract_matrix(const void *const data, const uint16_t element_bytes,
                           const uint64_t rows, const uint64_t cols,
                           const char *const filename, const bool compress) {
  if (data == NULL) {
    printf("write_abstract_matrix failed : %s\n", "data == NULL");
    return false;
  }

  if (filename == NULL || strlen(filename) <= 0) {
    printf("write_abstract_matrix failed : %s\n",
           "filename == NULL || strlen(filename) <= 0");
    return false;
  }

  if (element_bytes <= 0) {
    printf("write_abstract_matrix failed : %s\n", "element_bytes <= 0");
    return false;
  }

  if (rows * cols <= 0) {
    printf("write_abstract_matrix failed : %s\n", "rows * cols <= 0");
    return false;
  }

  if (compress) {
    gzFile zfile = gzopen(filename, "wb");
    if (zfile == NULL) {
      printf("write_abstract_matrix failed : %s\n", "zfile == NULL");
      return false;
    }
    gzfwrite(&rows, sizeof(uint64_t), 1, zfile);
    gzfwrite(&cols, sizeof(uint64_t), 1, zfile);

    gzfwrite(data, element_bytes, rows * cols, zfile);

    gzclose(zfile);
  } else {
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
      printf("write_abstract_matrix failed : %s\n", "file == NULL");
      return false;
    }

    fwrite(&rows, sizeof(uint64_t), 1, file);
    fwrite(&cols, sizeof(uint64_t), 1, file);
    fwrite(data, element_bytes, rows * cols, file);

    fclose(file);
  }

  return true;
}