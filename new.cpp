#pragma leco tool

import pixed;

int main() {
  auto ctx = pixed::create(256, 256);
  return pixed::write("atlas.png", ctx).map([] { return 0; }).log_error([] {
    return 1;
  });
}
