#include "userinput_render.h"
#include <burning_ship.h>
#include <filesystem>

#include <stdio.h>
#include <zlib.h>

#include <memory>

#include <iostream>
#include <mutex>
#include <renders.h>

using std::cout, std::endl;

using std::vector, std::string, std::shared_ptr;

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
  mat_ptrs(::render_method rm = render_method::age_linear)
      : mat_int16(new mat_age),
        mat_f32((rm != render_method::age_linear) ? (new mat_age_f32)
                                                  : nullptr),
        mat_cplxc3(nullptr) {
    if (mat_int16 == nullptr) {
      cout << "Failed to allocate memory." << endl;
      exit(1);
    }
    if ((rm != render_method::age_linear) && (mat_f32 == nullptr)) {
      cout << "Failed to allocate memory." << endl;
      exit(1);
    }
  }

  ~mat_ptrs() = default;

  shared_ptr<::mat_age> mat_int16;
  shared_ptr<::mat_age_f32> mat_f32;
  shared_ptr<::cplx_matc3> mat_cplxc3;
};

class obj_pool {
private:
  std::vector<std::pair<mat_ptrs, int>> pool;
  const render_method rm;
  std::mutex lock;

public:
  explicit obj_pool(const size_t size, const render_method rm) : rm(rm) {
    pool.reserve(size);

    while (pool.size() < size) {
      pool.emplace_back(std::make_pair(mat_ptrs(rm), -1));
    }
  }

  mat_ptrs allocate(const int id) {
    lock.lock();
    for (auto &pair : pool) {
      if (pair.second < 0) {
        pair.second = id;
        cout << "Allocated for id " << id << endl;
        lock.unlock();
        return pair.first;
      }
    }
    pool.emplace_back(std::make_pair(mat_ptrs(rm), id));
    cout << "Allocated for id " << id << endl;
    lock.unlock();
    return pool.back().first;
  }

  void deallocate(const int id) {
    lock.lock();
    for (auto &pair : pool) {
      if (pair.second == id) {
        pair.second = -1;
        continue;
      }
    }
    cout << "Deallocated for id " << id << endl;
    lock.unlock();
  }
};

void execute_rendering(const render_options &input) {

  obj_pool pool(input.threadnum * 2, input.method);

#pragma omp parallel for schedule(dynamic)
  for (int frameidx = 0; frameidx < input.sources.size(); frameidx++) {
    mat_ptrs mats = pool.allocate(frameidx);
    // compute
    // sleep(std::hash<int>()(frameidx) & 0xFFF);
    pool.deallocate(frameidx);
  }
}