#pragma leco app
#pragma leco add_impl atlased_atlasmode
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

void atlased::modes::sprite() {
  using namespace casein;

  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_ESCAPE, modes::atlas);
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