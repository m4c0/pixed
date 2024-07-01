module pixed;
import silog;

mno::req<void> pixed::read_ihdr(yoyo::reader &r, dec_ctx &ctx) {
  return r.read_u32_be()
      .map([&](auto n) { ctx.w = n; })
      .fmap([&] { return r.read_u32_be(); })
      .map([&](auto n) { ctx.h = n; })
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
        silog::log(silog::debug, "found %dx%d image", ctx.w, ctx.h);
      });
}
