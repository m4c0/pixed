#pragma leco app
#pragma leco add_impl atlased_atlasmode
#pragma leco add_impl atlased_sprmode
export module atlased;

import casein;
import dotz;
import pixed;
import quack;
import voo;

namespace atlased::modes {
void atlas();
void sprite();
} // namespace atlased::modes

pixed::context g_ctx = [] {
  auto res = pixed::create(256, 256);
  res.spr_size = {16, 16};

  for (auto &p : res.image) {
    p = {10, 40, 120, 255};
  }

  res.palette.set_capacity(4);
  res.palette[0] = {0, 0, 0, 0};
  res.palette[1] = {255, 255, 255, 255};
  res.palette[2] = {10, 40, 120, 255};
  res.palette[3] = {128, 128, 128, 255};

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

struct init {
  init() {
    using namespace quack::donald;

    app_name("atlas-editor");
    max_quads(10240);

    clear_colour({});
    atlas(::atlas);

    atlased::modes::atlas();
  }
} i;
