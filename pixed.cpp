#pragma leco app

import casein;
import quack;
import voo;

static quack::donald::atlas_t *atlas(voo::device_and_queue *dq) {
  return new voo::sires_image{"input.png", dq};
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
