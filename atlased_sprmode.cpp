module atlased;

static dotz::ivec2 g_cursor{};
static dotz::ivec2 g_sprite{};
static unsigned g_pal{};
static pixed::pixel g_brush{};

static unsigned data(quack::instance *i) {
  auto [sw, sh] = g_ctx.spr_size;
  dotz::vec2 sz = g_ctx.spr_size;

  quack::donald::push_constants({
      .grid_pos = sz / 2.0f,
      .grid_size = sz,
  });

  dotz::vec4 colour{g_brush.r, g_brush.g, g_brush.b, 255};
  *i++ = {
      .position = {-1, 0},
      .size = {1, 1},
      .colour = colour / 256.0,
  };

  *i++ = {
      .position = g_cursor,
      .size = {1, 1},
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
  return sw * sh + 1;
}

static void cursor(dotz::ivec2 d) {
  g_cursor = (g_cursor + d + g_ctx.spr_size) % g_ctx.spr_size;
  quack::donald::data(::data);
}

static void palette(int d) {
  g_pal = (g_pal + d) % g_ctx.palette.size();
  g_brush = g_ctx.palette[g_pal];
  quack::donald::data(::data);
}

static void tap() {
  auto c = g_sprite + g_cursor;
  auto p = c.y * g_ctx.w + c.x;
  g_ctx.image[p] = g_brush;
  atlased::load_atlas();
}

static void yank() {
  auto c = g_sprite + g_cursor;
  auto p = c.y * g_ctx.w + c.x;
  g_brush = g_ctx.image[p];
  quack::donald::data(::data);
}

void atlased::modes::sprite(dotz::ivec2 sel) {
  using namespace casein;

  g_sprite = sel * g_ctx.spr_size;

  handle(FILES_DROP, nullptr);
  reset_k(KEY_DOWN);

  if (g_ctx.palette.size() != 0) {
    handle(KEY_DOWN, K_Q, [] { palette(-1); });
    handle(KEY_DOWN, K_W, [] { palette(1); });
    g_brush = g_ctx.palette[0];
    g_pal = 0;
  } else {
    g_brush = {255, 255, 255};
  }

  handle(KEY_DOWN, K_Y, yank);
  handle(KEY_DOWN, K_SPACE, tap);

  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
  handle(KEY_DOWN, K_ESCAPE, modes::atlas);

  quack::donald::data(::data);
}
