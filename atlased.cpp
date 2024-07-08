#pragma leco app

import casein;
import pixed;
import quack;
import voo;

static pixed::context g_ctx = [] {
  auto res = pixed::create(256, 256);
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
  i->colour = {};
  i->multiplier = {1, 1, 1, 1};
  i->position = {0, 0};
  i->size = {1, 1};
  i->uv0 = {0, 0};
  i->uv1 = {1, 1};
  i->rotation = {};
  return 1;
}

struct init {
  init() {
    using namespace quack::donald;

    app_name("atlas-editor");
    max_quads(10240);

    clear_colour({});
    push_constants({
        .grid_pos = {0.5f, 0.5f},
        .grid_size = {1, 1},
    });
    atlas(::atlas);
    data(::data);
  }
} i;
