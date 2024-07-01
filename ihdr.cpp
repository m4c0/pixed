module pixed;
import fork;
import silog;

#pragma pack(push, 1)
namespace {
struct ihdr {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth = 8;
  uint8_t colour_type = 6; // RGBA
  uint8_t compression = 0; // Deflate
  uint8_t filter = 0;
  uint8_t interlace = 0;

  ihdr(uint32_t w, uint32_t h)
      : width{yoyo::flip32(w)}
      , height{yoyo::flip32(h)} {}
};
} // namespace
#pragma pack(pop)
static_assert(sizeof(ihdr) == 13);

mno::req<void> pixed::write_ihdr(yoyo::writer &wr, unsigned w, unsigned h) {
  return frk::chunk("IHDR", ihdr{w, h})(wr);
}

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
