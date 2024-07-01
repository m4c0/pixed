#pragma leco tool

import flate;
import fork;
import hai;
import pixed;
import silog;
import stubby;
import traits;
import yoyo;

using namespace traits::ints;
using namespace pixed;

int main() {
  dec_ctx img{};
  return pixed::read("blank.png")
      .map([&](auto &img) {
        auto d = reinterpret_cast<stbi::pixel *>(img.image.begin());
        stbi::write_rgba_unsafe("out/test.png", img.w, img.h, d);
        silog::log(silog::info, "recompressed successfully");
        return 0;
      })
      .log_error([] { throw 0; });
}
