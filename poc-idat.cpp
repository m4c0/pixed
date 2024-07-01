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
  yoyo::file_reader::open("blank.png")
      .fpeek(frk::assert("PNG"))
      .fpeek(read_ihdr(img))
      .fpeek(read_idat(img))
      .fpeek(frk::take("IEND"))
      .map(frk::end())
      .map([] { silog::log(silog::info, "decompressed successfully"); })
      .map([&] {
        auto d = reinterpret_cast<stbi::pixel *>(img.image.begin());
        stbi::write_rgba_unsafe("out/test.png", img.w, img.h, d);
      })
      .log_error([] { throw 0; });
}
