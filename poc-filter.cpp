#pragma leco tool

import pixed;

int main() {
  return pixed::read("blank.png")
      .fmap([](auto &ctx) {
        auto *sl = ctx.image.begin();
        for (auto y = 0; y < ctx.h; y++) {
          for (auto x = 0; x < ctx.w; x++, sl++) {
            sl->r = 256 * x / ctx.w;
            sl->b = 256 * y / ctx.h;
          }
        }
        return pixed::write("out/test.png", ctx);
      })
      .map([] { return 0; })
      .log_error([] { return 1; });
}
