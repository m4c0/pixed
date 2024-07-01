#pragma leco tool

import pixed;
import silog;

int main() {
  const pixed::pixel pal[]{
      {0, 0, 0, 255},
      {0, 0, 0, 0},
      {255, 255, 255, 255},
  };

  auto ctx = pixed::create(32, 16);
  pixed::set(ctx, 16, 7, pal[0]);
  pixed::set(ctx, 16, 8, pal[1]);
  pixed::set(ctx, 16, 9, pal[2]);
  return pixed::write("out/test.png", ctx).map([] { return 0; }).log_error([] {
    return 1;
  });
}
