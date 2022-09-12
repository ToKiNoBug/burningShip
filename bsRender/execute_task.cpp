#include "userinput_render.h"
#include <burning_ship.h>
#include <filesystem>

#include <stdio.h>
#include <zlib.h>

#include <array>
#include <cmath>
#include <colors.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <renders.h>
#include <unordered_map>

using std::cout, std::endl;

using std::vector, std::string, std::unique_ptr;

size_t read_matrix(const char *const filename, void *const dest,
                   const int size_element) {
  if (filename == nullptr || dest == nullptr) {
    return 0;
  }

  const std::filesystem::path path(filename);

  char buf[17];

  if (path.extension() == ".gz") {
    gzFile_s *const zfile = gzopen(filename, "rb");
    if (zfile == NULL) {
      return 0;
    }

    gzfread(buf, 1, 17, zfile);

    const size_t ret = gzfread(
        dest, 1, size_element * burning_ship_rows * burning_ship_cols, zfile);
    gzclose(zfile);
    return ret;
  } else {
    FILE *file = NULL;
    fopen_s(&file, filename, "rb");

    if (file == NULL) {
      return 0;
    }

    fread(buf, 1, 17, file);

    const size_t ret = fread(
        dest, 1, size_element * burning_ship_rows * burning_ship_cols, file);
    fclose(file);
    return ret;
  }
}

struct mat_ptrs {
  mat_ptrs(::render_method rm = render_method::age_linear,
           const size_t maxit = 3)
      : mat_int16(new mat_age),
        mat_f32((rm != render_method::age_linear) ? (new mat_age_f32)
                                                  : nullptr),
        mat_cplxc3(nullptr), f(maxit), row_ptrs_cache(burning_ship_rows),
        image(new ::std::array<pixel_u8c3,
                               burning_ship_rows * burning_ship_cols>) {
    if (mat_int16 == nullptr) {
      cout << "Failed to allocate memory for mat_int16." << endl;
      exit(1);
    }
    memset(&mat_int16->data[0][0], 0, sizeof(::mat_age));
    if ((rm != render_method::age_linear) && (mat_f32 == nullptr)) {
      cout << "Failed to allocate memory for mat_f32." << endl;
      exit(1);
    }
    if (rm != render_method::age_linear) {
      memset(&mat_f32->data[0][0], 0, sizeof(mat_age_f32));
    }

    if (f.data() == nullptr) {
      cout << "Failed to allocate memory for f." << endl;
      exit(1);
    }
    memset(f.data(), 0, sizeof(double) * f.size());

    if (row_ptrs_cache.data() == nullptr) {
      cout << "Failed to allocate memory for row pointers" << endl;
      exit(1);
    }
    memset(row_ptrs_cache.data(), 0, sizeof(void *) * row_ptrs_cache.size());

    if (image->data() == nullptr) {
      cout << "Failed to allocate memory for image (u8c3)" << endl;
      exit(1);
    }
    memset(image->data(), 0, sizeof(decltype(image)::element_type));
  }

  ~mat_ptrs() = default;

  unique_ptr<::mat_age> mat_int16;
  unique_ptr<::mat_age_f32> mat_f32;
  unique_ptr<::cplx_matc3> mat_cplxc3;
  vector<double> f;
  vector<const void *> row_ptrs_cache;
  unique_ptr<std::array<pixel_u8c3, burning_ship_rows * burning_ship_cols>>
      image;
};

class obj_pool {
private:
  // std::vector<std::pair<mat_ptrs, int>> pool;
  std::unordered_map<mat_ptrs *, bool> pool;
  const render_method rm;
  const size_t maxit;
  std::mutex lock;

public:
  explicit obj_pool(const size_t size, const render_method rm,
                    const size_t maxit)
      : rm(rm), maxit(maxit) {
    pool.reserve(size);

    while (pool.size() < size) {
      pool.emplace(new mat_ptrs(rm, maxit), false);
    }
  }

  // id is useless
  mat_ptrs *allocate(const int id = 0) {
    lock.lock();

    for (auto &pair : pool) {
      if (!pair.second) {
        pair.second = true;
        cout << "Allocated for id " << id << endl;
        lock.unlock();
        return pair.first;
      }
    }
    auto ret = pool.emplace(new mat_ptrs(rm, maxit), true);
    cout << "Allocated for id " << id << " with system call." << endl;
    lock.unlock();
    return ret.first->first;
  }

  inline bool is_from(mat_ptrs *ptr) { return pool.contains(ptr); }

  // id is useless
  void deallocate(mat_ptrs *ptr, const int id = 0) {
    lock.lock();

    auto it = pool.find(ptr);
    if (it != pool.end()) {
      it->second = false;
    }

    cout << "Deallocated for id " << id << endl;
    lock.unlock();
  }
};

// start to render
void execute_rendering(const render_options &input) {

  obj_pool pool(input.threadnum + 2, input.method, input.age_maxit);

  cout << "All memory allocation finished. Press enter to continue." << endl;

  getchar();

  const int max_digits = std::ceil(std::log10(input.png_count()) + 1e-2);

  std::mutex cout_lock;
  size_t tasks_finished = 0;
  size_t failed_count = 0;
  const size_t tasks = input.png_count();

#pragma omp parallel for schedule(dynamic)
  for (int frameidx = 0; frameidx < input.sources.size(); frameidx++) {
    mat_ptrs *const mats = pool.allocate(frameidx);
    thread_local string filename;
    filename.reserve(1024);

    // render
    for (int pngidx = 0; pngidx < input.fps; pngidx++) {
      // set the filename
      filename = input.dest_prefix;
      const int current_number = frameidx * input.fps + pngidx;

      const int current_number_digits =
          (current_number == 0) ? 1
                                : std::ceil(std::log10(current_number) + 1e-9);

      filename.append(max_digits - current_number_digits, '0');

      filename += std::to_string(current_number);
      filename += ".png";

      // render the frame
      if ((pngidx == 0) && (input.method == render_method::age_linear)) {
        ::render_u8c1(mats->mat_int16.get(),
                      (uint8_t *)mats->image.get()->data(), input.age_maxit);
      }
      // compute the image size
      const double pngidx_div_fps = double(pngidx) / input.fps;

      const double rows_div_max_rows = std::pow(
          input.zoomspeed,
          (input.zoomspeed > 1) ? (-pngidx_div_fps) : (1.0 - pngidx_div_fps));

      const int skip_rows =
          std::round((burning_ship_rows * (1.0 - rows_div_max_rows) / 2.0));

      const int skip_cols =
          std::round((burning_ship_cols * (1.0 - rows_div_max_rows) / 2.0));

      const int image_rows = burning_ship_rows - 2 * skip_rows;
      const int image_cols = burning_ship_cols - 2 * skip_cols;
      bool ok = false;
      // render as u8c1 linear
      if (input.method == render_method::age_linear) {
        // the first address of this matrix
        const uint8_t *const first_address =
            (uint8_t *)mats->image.get()->data();

        mats->row_ptrs_cache.resize(burning_ship_rows - 2 * skip_rows);

        for (int r = 0; r < image_rows; r++) {
          mats->row_ptrs_cache[r] =
              first_address + (skip_rows + r) * burning_ship_cols + skip_cols;
        }

        ok = ::write_png_u8c1_rowptrs(
            (const uint8_t *const *const)mats->row_ptrs_cache.data(),
            image_rows, image_cols, filename.data());
      }

      cout_lock.lock();
      if (ok) {
        tasks_finished++;
        cout << "[ " << tasks_finished << " / " << tasks << " , "
             << float(tasks_finished) / tasks << " ] : Generated file "
             << filename << endl;
      } else {
        failed_count++;
        cout << "Failed to generate file " << endl;
      }
      cout_lock.unlock();
    }
    pool.deallocate(mats, frameidx);
  }
}