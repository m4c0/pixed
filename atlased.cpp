#pragma leco app

import casein;
import dotz;
import pixed;
import quack;
import voo;

static pixed::context g_ctx = [] {
  auto res = pixed::create(256, 256);
  res.spr_size = {16, 16};
  for (auto &p : res.image) {
    p = {10, 40, 120, 255};
  }
  return res;
}();

static void atlas(voo::h2l_image *img) {
  voo::mapmem m{img->host_memory()};
  auto *c = static_cast<pixed::pixel *>(*m);
  for (auto &p : g_ctx.image)
    *c++ = p;
}
static voo::updater<voo::h2l_image> *atlas(voo::device_and_queue *dq) {
  return new voo::updater<voo::h2l_image>{
      dq->queue(), atlas, dq->physical_device(), g_ctx.w, g_ctx.h};
}

static unsigned data(quack::instance *i) {
  auto [sw, sh] = g_ctx.spr_size;
  dotz::vec2 sz = g_ctx.spr_size;

  quack::donald::push_constants({
      .grid_pos = sz / 2.0f,
      .grid_size = sz,
  });

  for (dotz::vec2 p{}; p.y < sh; p.y++) {
    for (p.x = 0; p.x < sw; p.x++) {
      *i++ = {
          .position = p,
          .size = dotz::vec2(0.9f),
          .uv0 = p / sz,
          .uv1 = (p + 1) / sz,
          .multiplier = {1, 1, 1, 1},
      };
    }
  }
  return sw * sh;
}

struct init {
  init() {
    using namespace quack::donald;

    app_name("atlas-editor");
    max_quads(10240);

    clear_colour({});
    atlas(::atlas);
    data(::data);
  }
} i;
