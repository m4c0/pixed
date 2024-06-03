#pragma leco app

import casein;
import quack;
import traits;
import voo;

using namespace traits::ints;

static void update_atlas(voo::h2l_image *img) {
  voo::mapmem m{img->host_memory()};
  auto *buf = static_cast<uint32_t *>(*m);
  for (auto y = 0; y < 16; y++) {
    for (auto x = 0; x < 16; x++, buf++) {
      *buf = 0U;
    }
  }
}
static quack::donald::atlas_t *atlas(voo::device_and_queue *dq) {
  return new quack::donald::atlas_t{dq->queue(), &update_atlas,
                                    dq->physical_device(), 16U, 16U};
}

static unsigned quads_4x4(quack::mapped_buffers all) {
  auto [c, m, p, u] = all;
  for (auto y = 0, i = 0; y < 4; y++) {
    for (auto x = 0; x < 4; x++, i++) {
      float xf = x + 0.05;
      float yf = y + 0.05;
      c[i] = {1, 1, 1, 1};
      m[i] = {1, 1, 1, 1};
      p[i] = {{xf, yf}, {0.9, 0.9}};
      u[i] = {{0, 0}, {1, 1}};
    }
  }
  return 16;
}

struct init {
  init() {
    using namespace quack::donald;

    app_name("pixed");
    max_quads(16);

    push_constants({
        .grid_pos = {2, 2},
        .grid_size = {4, 4},
    });
    atlas(::atlas);
    data(quads_4x4);
  }
} i;
