module atlased;

static unsigned g_bit{};
static pixed::pixel *g_target;
static pixed::pixel g_original;
static unsigned (*g_fn)(quack::instance *i);

static void type(unsigned n) {
  switch (g_bit++) {
  case 0:
    g_target->r = (n << 4) | n;
    break;
  case 1:
    g_target->g = (n << 4) | n;
    break;
  case 2:
    g_target->b = (n << 4) | n;
    break;
  case 3:
    g_target->a = (n << 4) | n;
    atlased::modes::sprite();
    break;
  }

  quack::donald::data(g_fn);
};

static void reset() {
  *g_target = g_original;
  atlased::modes::sprite();
}

void atlased::modes::colour(pixed::pixel *p,
                            unsigned (*fn)(quack::instance *)) {
  using namespace casein;

  g_bit = 0;
  g_target = p;
  g_fn = fn;
  g_original = *p;

  handle(FILES_DROP, nullptr);
  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_0, [] { type(0x0); });
  handle(KEY_DOWN, K_1, [] { type(0x1); });
  handle(KEY_DOWN, K_2, [] { type(0x2); });
  handle(KEY_DOWN, K_3, [] { type(0x3); });
  handle(KEY_DOWN, K_4, [] { type(0x4); });
  handle(KEY_DOWN, K_5, [] { type(0x5); });
  handle(KEY_DOWN, K_6, [] { type(0x6); });
  handle(KEY_DOWN, K_7, [] { type(0x7); });
  handle(KEY_DOWN, K_8, [] { type(0x8); });
  handle(KEY_DOWN, K_9, [] { type(0x9); });
  handle(KEY_DOWN, K_A, [] { type(0xA); });
  handle(KEY_DOWN, K_B, [] { type(0xB); });
  handle(KEY_DOWN, K_C, [] { type(0xC); });
  handle(KEY_DOWN, K_D, [] { type(0xD); });
  handle(KEY_DOWN, K_E, [] { type(0xE); });
  handle(KEY_DOWN, K_F, [] { type(0xF); });

  handle(KEY_DOWN, K_ESCAPE, reset);
}
