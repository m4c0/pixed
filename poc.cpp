#pragma leco tool

import pixed;
import silog;

int main() {
  auto ctx = pixed::create(32, 16);
  ctx.palette.set_capacity(3);
  ctx.palette[0] = {0, 0, 0, 255};
  ctx.palette[1] = {0, 0, 0, 0};
  ctx.palette[2] = {255, 255, 255, 255};

  pixed::at(ctx, 16, 7) = ctx.palette[0];
  pixed::at(ctx, 16, 8) = ctx.palette[1];
  pixed::at(ctx, 16, 9) = ctx.palette[2];
  return pixed::write("out/test.png", ctx).map([] { return 0; }).log_error([] {
    return 1;
  });
}
