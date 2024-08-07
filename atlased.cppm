#pragma leco app
#pragma leco add_impl atlased_atlasmode
#pragma leco add_impl atlased_config
#pragma leco add_impl atlased_colourmode
#pragma leco add_impl atlased_sprmode
export module atlased;

import casein;
import dotz;
import jute;
import missingno;
import pixed;
import quack;
import voo;

namespace atlased::modes {
void atlas();
void colour(pixed::pixel *p, unsigned (*fn)(quack::instance *));
void sprite(dotz::ivec2 p);
void sprite();
} // namespace atlased::modes

namespace atlased::config {
void load();
const char *current_file();
mno::req<void> set_current_file(jute::view);
} // namespace atlased::config

pixed::context g_ctx = [] {
  auto res = pixed::create(256, 256);
  res.spr_size = {16, 16};

  for (auto &p : res.image) {
    p = {10, 40, 120, 255};
  }

  res.palette.set_capacity(4);
  res.palette[0] = {0, 0, 0, 255};
  res.palette[1] = {255, 255, 255, 255};
  res.palette[2] = {10, 40, 120, 255};
  res.palette[3] = {128, 128, 128, 255};

  return res;
}();

namespace atlased {
void reset_casein() {
  using namespace casein;

  handle(FILES_DROP, nullptr);
  reset_k(KEY_DOWN);
  reset_k(KEY_UP);
}

inline dotz::ivec2 image_size() { return dotz::ivec2{g_ctx.w, g_ctx.h}; }
inline dotz::ivec2 grid_size() { return image_size() / g_ctx.spr_size; }

void load_atlas() { quack::donald::atlas(g_ctx.image.begin(), g_ctx.w, g_ctx.h); }
dotz::ivec2 mark();
} // namespace atlased

struct init {
  init() {
    using namespace quack::donald;

    app_name("atlas-editor");
    max_quads(10240);

    clear_colour({});

    atlased::config::load();
    atlased::load_atlas();
    atlased::modes::atlas();
  }
} i;
