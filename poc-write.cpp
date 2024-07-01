#pragma leco tool
import pixed;

int main() {
  static constexpr const auto w = 1024;
  static constexpr const auto h = 1024;
  auto ctx = pixed::create(w, h);
  auto *sl = ctx.image.begin();
  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++, sl += 4) {
      sl[0] = sl[3] = 0xFF;
      sl[1] = 256 * x / w;
      sl[2] = 256 * y / h;
    }
  }

  return pixed::write("out/test.png", ctx)
          .map([] { return 0; })
          .log_error([] { return 1; });
}
