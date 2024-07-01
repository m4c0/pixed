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

static constexpr auto ihdr(dec_ctx &ctx) {
  return [ctx = &ctx](yoyo::subreader r) {
    return r.read_u32_be()
        .map([&](auto n) { ctx->w = n; })
        .fmap([&] { return r.read_u32_be(); })
        .map([&](auto n) { ctx->h = n; })
        .fmap([&] { return r.read_u8(); })
        .assert([](auto bit_depth) { return bit_depth == 8; },
                "unsupported bitdepth")
        .fmap([&](auto) { return r.read_u8(); })
        .assert([](auto colour_type) { return colour_type == 6; },
                "unsupported colour type")
        .fmap([&](auto) { return r.read_u8(); })
        .assert([](auto compression) { return compression == 0; },
                "unsupported compression")
        .fmap([&](auto) { return r.read_u8(); })
        .assert([](auto filter) { return filter == 0; }, "unsupported filter")
        .fmap([&](auto) { return r.read_u8(); })
        .assert([](auto interlace) { return interlace == 0; },
                "unsupported interlace")
        .map([&](auto) {
          silog::log(silog::debug, "found %dx%d image", ctx->w, ctx->h);
        });
  };
}

int main() {
  dec_ctx img{};
  yoyo::file_reader::open("blank.png")
      .fpeek(frk::assert("PNG"))
      .fpeek(frk::take("IHDR", ihdr(img)))
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
