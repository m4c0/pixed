module atlased;

static dotz::ivec2 g_cursor{};

static unsigned data(quack::instance *i) {
  auto [sw, sh] = atlased::grid_size();
  dotz::vec2 sz = atlased::grid_size();

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
  auto gs = atlased::grid_size();
  g_cursor = (g_cursor + d + gs) % gs;
  quack::donald::data(::data);
}

static void spr_size(dotz::ivec2 d) {
  do {
    g_ctx.spr_size = g_ctx.spr_size + d;
  } while (dotz::sq_length(atlased::image_size() % g_ctx.spr_size) != 0);

  g_cursor = dotz::min(g_cursor, atlased::grid_size() - 1);
  quack::donald::data(::data);
}

void atlased::modes::atlas() {
  using namespace casein;

  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_A, [] { spr_size({1, 0}); });
  handle(KEY_DOWN, K_D, [] { spr_size({-1, 0}); });
  handle(KEY_DOWN, K_W, [] { spr_size({0, 1}); });
  handle(KEY_DOWN, K_S, [] { spr_size({0, -1}); });

  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
  handle(KEY_DOWN, K_ENTER, [] { modes::sprite(g_cursor); });

  quack::donald::data(::data);
}
