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

static dotz::ivec2 g_cursor{};

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

  *i++ = {
      .position = g_cursor,
      .size = {1, 1},
      .colour = {1, 0, 0, 1},
  };

  for (dotz::vec2 p{}; p.y < sh; p.y++) {
    for (p.x = 0; p.x < sw; p.x++) {
      *i++ = {
          .position = p + 0.05f,
          .size = dotz::vec2(0.9f),
          .uv0 = p / sz,
          .uv1 = (p + 1) / sz,
          .multiplier = {1, 1, 1, 1},
      };
    }
  }
  return sw * sh + 1;
}

static void cursor(dotz::ivec2 d) {
  g_cursor = (g_cursor + d + g_ctx.spr_size) % g_ctx.spr_size;
  quack::donald::data(::data);
}

static void open_sprite();

static void open_atlas() {
  using namespace casein;

  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
  handle(KEY_DOWN, K_ENTER, open_sprite);
}
static void open_sprite() {
  using namespace casein;

  reset_k(KEY_DOWN);
}

struct init {
  init() {
    open_atlas();

    using namespace quack::donald;

    app_name("atlas-editor");
    max_quads(10240);

    clear_colour({});
    atlas(::atlas);
    data(::data);
  }
} i;
