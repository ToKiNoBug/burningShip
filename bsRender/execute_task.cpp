#include "userinput_render.h"
#include <burning_ship.h>
#include <filesystem>

#include <stdio.h>
#include <zlib.h>

#include <array>
#include <cmath>
#include <colors.h>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <renders.h>
#include <unordered_map>

#include <omp.h>

#include <memory.h>

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
    file=fopen(filename, "rb");

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
      : mat_int16(new mat_age), mat_norm2(new norm2_matc1),
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

    if (rm == render_method::age_norm2_q || rm == render_method::norm2_only) {
      if (mat_norm2 == nullptr) {
        cout << "Failed to allocate memory for mat_norm2." << endl;
        exit(1);
      }
      memset(&mat_norm2->norm2[0][0], 0, sizeof(norm2_matc1));
    }

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
  unique_ptr<::norm2_matc1> mat_norm2;
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
        // cout << "Allocated for id " << id << endl;
        lock.unlock();
        return pair.first;
      }
    }
    auto ret = pool.emplace(new mat_ptrs(rm, maxit), true);
    // cout << "Allocated for id " << id << " with system call." << endl;
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

    // cout << "Deallocated for id " << id << endl;
    lock.unlock();
  }
};

// start to render
void execute_rendering(const render_options &input) {

  obj_pool pool(input.threadnum + 2, input.method, input.age_maxit);

  const int max_digits = std::ceil(std::log10(input.png_count()) + 1e-2);

  // std::mutex cout_lock;
  size_t tasks_finished = 0;
  size_t failed_count = 0;
  const size_t tasks = input.png_count();

  omp_lock_t lock;
  omp_init_lock(&lock);

  std::clock_t time = std::clock();

#pragma omp parallel for schedule(dynamic)
  for (int frameidx = 0; frameidx < input.sources.size(); frameidx++) {

    mat_ptrs *const mats = pool.allocate(frameidx);

    const auto &sources = input.sources[frameidx];

    read_matrix(sources.bs_frame.data(), &mats->mat_int16->data[0][0],
                sizeof(int16_t));

    if (!sources.bs_norm2.empty()) {
      read_matrix(sources.bs_norm2.data(), &mats->mat_norm2->norm2[0][0],
                  sizeof(bs_float));
    }

    if (!sources.bs_cplx_c3.empty()) {
      read_matrix(sources.bs_cplx_c3.data(), &mats->mat_cplxc3->c3[0][0][0],
                  sizeof(bs_cplx[3]));
    }

    thread_local string filename;
    filename.reserve(1024);

    bool smooth_norm2;
    bool color_by_norm2;
    bool render_age_only;
    bool write_gray;
    bool use_q_method;

    switch (input.method) {
    case render_method::age_linear:
      smooth_norm2 = false;
      color_by_norm2 = false;
      write_gray = true;
      render_age_only = true;
      use_q_method = false;
      break;
    case render_method::age_norm2_q:
      smooth_norm2 = true;
      color_by_norm2 = false;
      write_gray = false;
      render_age_only = false;
      use_q_method = true;
      break;
    case render_method::norm2_only:
      smooth_norm2 = true;
      color_by_norm2 = true;
      write_gray = false;
      render_age_only = false;
      use_q_method = false;
      break;
    case render_method::age_q:
      smooth_norm2 = false;
      color_by_norm2 = false;
      write_gray = false;
      render_age_only = true;
      use_q_method = true;
      break;
    }

    // render the frame
    if (write_gray) {
      render_u8c1(mats->mat_int16.get(), (uint8_t *)mats->image.get()->data(),
                  input.age_maxit);
    }

    if (smooth_norm2) {
      smooth_by_norm2(mats->mat_int16.get(), mats->mat_norm2.get(),
                      mats->mat_f32.get());
    }

    if (color_by_norm2) {
      coloring_by_f32_u8c3(mats->mat_int16.get(), mats->mat_f32.get(),
                           mats->image->data());
    }

    thread_local ::render_by_q_options q_opts;

    if (use_q_method) {
      // do no rendering here cause rendering can't be done ahead of time.
      // set the value of q_opts.
      q_opts.newton_max_it = input.render_maxit;
      q_opts.err_tolerence = 1e-5;
      q_opts.f_buffer = mats->f.data();
      q_opts.L_mean_div_L_max = input.lightness;
      // don't guess at the first time.
      q_opts.q_guess = -1;

      // these members are determined by skip_rows and skip_cols;
      // q_opts.hist_skip_cols;
      // q_opts.hist_skip_rows;
    }

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

      if (use_q_method) {
        q_opts.hist_skip_cols = skip_cols;
        q_opts.hist_skip_rows = skip_rows;
        double q;
        ::smooth_age_by_q(mats->mat_int16.get(),
                          (render_age_only) ? (nullptr) : (mats->mat_f32.get()),
                          input.age_maxit, &q_opts, mats->mat_f32.get(), &q,
                          nullptr);
        q_opts.q_guess = q;
        if (!input.self_adaptive_f32) {
          ::coloring_by_f32_u8c3(mats->mat_int16.get(), mats->mat_f32.get(),
                                 mats->image->data());
        } else {
          ::coloring_by_f32_u8c3_more(mats->mat_int16.get(),
                                      mats->mat_f32.get(), mats->image->data(),
                                      NAN, NAN);
        }
      }

      // export as u8c1
      if (write_gray) {
        // the first address of this matrix
        const uint8_t *const first_address =
            (uint8_t *)mats->image.get()->data();

        mats->row_ptrs_cache.resize(image_rows);

        for (int r = 0; r < image_rows; r++) {
          mats->row_ptrs_cache[r] =
              first_address + (skip_rows + r) * burning_ship_cols + skip_cols;
        }
        for (int r = mats->row_ptrs_cache.size();
             r < mats->row_ptrs_cache.capacity(); r++) {
          const void **ptrptr = mats->row_ptrs_cache.data();
          ptrptr[r] = nullptr;
        }

        ok = ::write_png_u8c1_rowptrs(
            (const uint8_t *const *const)mats->row_ptrs_cache.data(),
            image_rows, image_cols, filename.data());
      }
      // export as u8c3
      if (!write_gray) {
        const pixel_u8c3 *const first_address = mats->image->data();

        mats->row_ptrs_cache.resize(image_rows);

        for (int r = 0; r < image_rows; r++) {
          mats->row_ptrs_cache[r] =
              first_address + (skip_rows + r) * burning_ship_cols + skip_cols;
        }
        for (int r = mats->row_ptrs_cache.size();
             r < mats->row_ptrs_cache.capacity(); r++) {
          const void **ptrptr = mats->row_ptrs_cache.data();
          ptrptr[r] = nullptr;
        }
        ok = ::write_png_u8c3_rowptrs(mats->row_ptrs_cache.data(), image_rows,
                                      image_cols, filename.data());
      }

      omp_set_lock(&lock);
      if (ok) {
        tasks_finished++;
        cout << "[ " << tasks_finished << " / " << tasks << " , "
             << float(tasks_finished) / tasks << " ] : Generated file "
             << filename << endl;
      } else {
        failed_count++;
        cout << "Failed to generate file " << endl;
      }
      omp_unset_lock(&lock);
    }

    // omp_set_lock(&lock);
    pool.deallocate(mats, frameidx);
    // omp_unset_lock(&lock);
  }

  omp_destroy_lock(&lock);

  time = std::clock() - time;

  cout << "All tasks finished with " << failed_count << " error(s)" << endl;
  cout << "Computation cost " << double(time) / CLOCKS_PER_SEC << " seconds."
       << endl;
}