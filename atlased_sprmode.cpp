module atlased;

static dotz::ivec2 g_cursor{};
static unsigned g_pal{};

static unsigned data(quack::instance *i) {
  auto [sw, sh] = g_ctx.spr_size;
  dotz::vec2 sz = g_ctx.spr_size;

  quack::donald::push_constants({
      .grid_pos = sz / 2.0f,
      .grid_size = sz,
  });

  dotz::vec4 colour{255, 255, 255, 255};
  if (g_ctx.palette.size() > 0) {
    auto pal = g_ctx.palette[g_pal];
    colour = {pal.r, pal.g, pal.b, 255};
  }
  colour = colour / 256.0;

  *i++ = {
      .position = g_cursor,
      .size = {1, 1},
      .colour = colour,
  };

  for (dotz::vec2 p{}; p.y < sh; p.y++) {
    for (p.x = 0; p.x < sw; p.x++) {
      *i++ = {
          .position = p + 0.05f,
          .size = dotz::vec2(0.9f),
          .uv0 = p / atlased::image_size(),
          .uv1 = (p + 1) / atlased::image_size(),
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
  quack::donald::data(::data);
}

static void tap() {
  auto p = g_cursor.y * g_ctx.w + g_cursor.x;
  g_ctx.image[p] = g_ctx.palette[g_pal];
  atlased::load_atlas();
}

void atlased::modes::sprite() {
  using namespace casein;

  reset_k(KEY_DOWN);

  if (g_ctx.palette.size() != 0) {
    handle(KEY_DOWN, K_Q, [] { palette(-1); });
    handle(KEY_DOWN, K_W, [] { palette(1); });
    handle(KEY_DOWN, K_SPACE, tap);
  }

  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
  handle(KEY_DOWN, K_ESCAPE, modes::atlas);

  quack::donald::data(::data);
}

