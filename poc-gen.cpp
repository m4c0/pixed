#pragma leco tool

import pixed;

int main() {
  auto ctx = pixed::create(256, 256);
  auto *p = ctx.image.begin();
  for (auto y = 0; y < 256; y++) {
    for (auto x = 0; x < 256; x++, p++) {
      p->r = p->a = 255;
      p->g = x;
      p->b = y;
    }
  }
  return pixed::write("out/test.png", ctx).map([] { return 0; }).log_error([] {
    return 1;
  });
}
