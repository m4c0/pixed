#pragma leco tool
import fork;
import hai;
import traits;
import yoyo;

using namespace traits::ints;

#pragma pack(push, 1)
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
#pragma pack(pop)
static_assert(sizeof(ihdr) == 13);

static constexpr auto adler(uint8_t *data, unsigned len) {
  static constexpr const auto base = 65521;
  uint32_t s1 = 1;
  uint32_t s2 = 0;
  for (auto i = 0; i < len; i++) {
    s1 = (s1 + data[i]) % base;
    s2 = (s2 + s1) % base;
  }
  return (s2 << 16) + s1;
}

static constexpr auto write_u8(uint8_t n) {
  return [=](auto &w) { return w.write_u8(n); };
}
static constexpr auto write_u16(uint16_t n) {
  return [=](auto &w) { return w.write_u16(n); };
}
static constexpr auto write(const void *data, unsigned sz) {
  return [=](auto &w) { return w.write(data, sz); };
}
static constexpr auto write_u32_be(uint16_t n) {
  return [=](auto &w) { return w.write_u32_be(n); };
}

int main() {
  static constexpr const auto w = 32;
  static constexpr const auto h = 16;
  static constexpr const auto img_len = 4 * h * (w + 1);

  hai::array<uint8_t> pixels{img_len};
  auto *sl = pixels.begin();
  for (auto y = 0; y < h; y++) {
    *sl++ = 0; // filter 0
    for (auto x = 0; x < w; x++, sl += 4) {
      sl[0] = sl[3] = 0xFF;
      sl[1] = 256 * x / w;
      sl[2] = 256 * y / h;
    }
  }

  hai::array<uint8_t> buf{1 << 13};
  unsigned len{};
  return mno::req{yoyo::memwriter{buf}}
      .fpeek(write_u8(0x78))      // CMF
      .fpeek(write_u8(0x01))      // FLG
      .fpeek(write_u8(1))         // BHEAD
      .fpeek(write_u16(img_len))  // LEN
      .fpeek(write_u16(~img_len)) // NLEN
      .fpeek(write(pixels.begin(), pixels.size()))
      .fpeek(write_u32_be(adler(pixels.begin(), pixels.size())))
      .fmap([&](auto &w) { return w.tellp(); })
      .map([&](auto l) { len = l; })
      .fmap([] { return yoyo::file_writer::open("out/test.png"); })
      .fmap(frk::signature("PNG"))
      .fmap(frk::chunk("IHDR", ihdr{w, h}))
      .fmap(frk::chunk("IDAT", buf.begin(), buf.size()))
      .fmap(frk::chunk("IEND"))
      .map(frk::end())
      .map([] { return 0; })
      .log_error([] { return 1; });
}
