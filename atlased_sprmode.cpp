module atlased;

static dotz::ivec2 g_cursor{};
static dotz::ivec2 g_cursor_e{};
static dotz::ivec2 g_sprite{};
static unsigned g_pal{};
static pixed::pixel g_brush{255, 255, 255, 255};
static bool g_area{};

static auto area() {
  auto s = dotz::min(g_cursor, g_cursor_e);
  auto e = dotz::max(g_cursor, g_cursor_e);
  struct pair {
    dotz::ivec2 s, e;
  };
  return pair{s, e};
}

static auto idx(unsigned x, unsigned y) {
  auto c = g_sprite + dotz::ivec2{x, y};
  return c.y * g_ctx.w + c.x;
}

static void draw_brush(pixed::pixel bruh, dotz::vec2 pos, quack::instance *&i) {
  dotz::vec4 colour{bruh.r, bruh.g, bruh.b, bruh.a};
  *i++ = {
      .position = pos,
      .size = {1, 1},
      .colour = colour / 256.0,
  };
}

static unsigned data(quack::instance *i) {
  auto [sw, sh] = g_ctx.spr_size;
  dotz::vec2 sz = g_ctx.spr_size;

  quack::donald::push_constants({
      .grid_pos = sz / 2.0f,
      .grid_size = sz,
  });
  auto pp = i;

  draw_brush(g_brush, {-3, static_cast<int>(g_pal)}, i);

  for (auto idx = 0; idx < g_ctx.palette.size(); idx++) {
    draw_brush(g_ctx.palette[idx], {-2, idx}, i);
  }

  // Drawing area contour {{{
  *i++ = {
      .position = dotz::vec2{-0.1f},
      .size = sz + 0.2f,
      .colour = {1, 1, 1, 1},
  };
  *i++ = {
      .position = dotz::vec2{},
      .size = sz,
      .colour = {0, 0, 0, 1},
  };
  // }}}

  // Miniature in a grid {{{
  dotz::vec2 spr = g_sprite;
  for (dotz::vec2 p{}; p.y < 3; p.y++) {
    for (p.x = 0; p.x < 3; p.x++) {
      *i++ = {
          .position = p + dotz::vec2{sz.x + 1.0f, 0.0f},
          .size{1},
          .uv0 = spr / atlased::image_size(),
          .uv1 = (spr + sz) / atlased::image_size(),
          .multiplier{1, 1, 1, 1},
      };
    }
  }
  // }}}

  auto [s, e] = area();
  *i++ = {
      .position = s,
      .size = e - s + 1,
      .colour = {1, 1, 1, 1},
  };

  for (dotz::vec2 p{}; p.y < sh; p.y++) {
    for (p.x = 0; p.x < sw; p.x++) {
      auto uv = g_sprite + p;
      *i++ = {
          .position = p + 0.05f,
          .size = dotz::vec2(0.9f),
          .uv0 = uv / atlased::image_size(),
          .uv1 = (uv + 1) / atlased::image_size(),
          .multiplier = {1, 1, 1, 1},
      };
    }
  }
  return i - pp;
}

static void cursor(dotz::ivec2 d) {
  g_cursor = (g_cursor + d + g_ctx.spr_size) % g_ctx.spr_size;
  if (!g_area)
    g_cursor_e = g_cursor;
  quack::donald::data(::data);
}

static void palette(int d) {
  g_pal = (g_pal + d) % g_ctx.palette.size();
  g_brush = g_ctx.palette[g_pal];
  quack::donald::data(::data);
}

static void tap() {
  auto [s, e] = area();
  for (auto y = s.y; y <= e.y; y++) {
    for (auto x = s.x; x <= e.x; x++) {
      g_ctx.image[idx(x, y)] = g_brush;
    }
  }
  atlased::load_atlas();
}

static void yank() {
  auto c = g_sprite + g_cursor;
  auto p = c.y * g_ctx.w + c.x;
  g_brush = g_ctx.image[p];
  quack::donald::data(::data);
}

static void delete_pal() {
  for (auto i = g_pal; i < g_ctx.palette.size() - 1; i++) {
    g_ctx.palette[i] = g_ctx.palette[i + 1];
  }
  g_ctx.palette.pop_back();
  if (g_pal == g_ctx.palette.size()) g_pal--;
  quack::donald::data(::data);
}
static void change_pal() {
  g_ctx.palette[g_pal] = g_brush;
  quack::donald::data(::data);
}
static void append_pal() {
  g_ctx.palette.add_capacity(1);
  g_pal = g_ctx.palette.size() - 1;
  change_pal();
}

static void move_sprite_l() {
  for (auto y = 0; y < g_ctx.spr_size.y; y++) {
    for (auto x = 0; x < g_ctx.spr_size.x - 1; x++) {
      auto p = idx(x, y);
      g_ctx.image[p] = g_ctx.image[p + 1];
    }
  }
  atlased::load_atlas();
}
static void move_sprite_r() {
  for (auto y = 0; y < g_ctx.spr_size.y; y++) {
    for (auto x = g_ctx.spr_size.x - 1; x > 0; x--) {
      auto p = idx(x, y);
      g_ctx.image[p] = g_ctx.image[p - 1];
    }
  }
  atlased::load_atlas();
}
static void move_sprite_u() {
  for (auto y = 0; y < g_ctx.spr_size.y - 1; y++) {
    for (auto x = 0; x < g_ctx.spr_size.x; x++) {
      auto p = idx(x, y);
      g_ctx.image[p] = g_ctx.image[p + g_ctx.w];
    }
  }
  atlased::load_atlas();
}
static void move_sprite_d() {
  for (auto y = g_ctx.spr_size.y - 1; y > 0; y--) {
    for (auto x = 0; x < g_ctx.spr_size.x; x++) {
      auto p = idx(x, y);
      g_ctx.image[p] = g_ctx.image[p - g_ctx.w];
    }
  }
  atlased::load_atlas();
}

void atlased::modes::sprite(dotz::ivec2 sel) {
  g_area = false;
  g_sprite = sel * g_ctx.spr_size;

  sprite();
}

static void move_cursor() {
  using namespace casein;
  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
}
static void move_sprite() {
  using namespace casein;
  handle(KEY_DOWN, K_LEFT, move_sprite_l);
  handle(KEY_DOWN, K_RIGHT, move_sprite_r);
  handle(KEY_DOWN, K_UP, move_sprite_u);
  handle(KEY_DOWN, K_DOWN, move_sprite_d);
}

void atlased::modes::sprite() {
  using namespace casein;

  handle(FILES_DROP, nullptr);
  reset_k(KEY_DOWN);

  if (g_ctx.palette.size() != 0) {
    handle(KEY_DOWN, K_Q, [] { palette(-1); });
    handle(KEY_DOWN, K_W, [] { palette(1); });
  }
  handle(KEY_DOWN, K_A, append_pal);
  handle(KEY_DOWN, K_C, change_pal);
  handle(KEY_DOWN, K_D, delete_pal);

  handle(KEY_DOWN, K_Y, yank);
  handle(KEY_DOWN, K_SPACE, tap);

  handle(KEY_DOWN, K_COMMA, [] { atlased::modes::colour(&g_brush, ::data); });

  handle(KEY_DOWN, K_V, [] { g_area = true; });
  handle(KEY_UP, K_V, [] { g_area = false; });

  handle(KEY_DOWN, K_M, move_sprite);
  handle(KEY_UP, K_M, move_cursor);

  move_cursor();
  handle(KEY_DOWN, K_ESCAPE, modes::atlas);

  quack::donald::data(::data);
}
