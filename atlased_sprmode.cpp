module atlased;

static unsigned data(quack::instance *i) { return 0; }

void atlased::modes::sprite() {
  using namespace casein;

  reset_k(KEY_DOWN);

  handle(KEY_DOWN, K_ESCAPE, modes::atlas);

  quack::donald::data(::data);
}

