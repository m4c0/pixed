module atlased;

static dotz::ivec2 g_cursor{};

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

void atlased::modes::atlas() {
  using namespace casein;

  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
  handle(KEY_DOWN, K_ENTER, modes::sprite);

  quack::donald::data(::data);
}
