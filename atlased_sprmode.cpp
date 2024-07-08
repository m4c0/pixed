module atlased;

static dotz::ivec2 g_cursor{};

static unsigned data(quack::instance *i) { return 0; }

static void cursor(dotz::ivec2 d) {
  g_cursor = (g_cursor + d + g_ctx.spr_size) % g_ctx.spr_size;
  quack::donald::data(::data);
}

void atlased::modes::sprite() {
  using namespace casein;

  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_LEFT, [] { cursor({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { cursor({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { cursor({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { cursor({0, 1}); });
  handle(KEY_DOWN, K_ESCAPE, modes::atlas);

  quack::donald::data(::data);
}

