#pragma leco tool
import fork;
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

static constexpr const auto w = 32;
static constexpr const auto h = 16;
static constexpr const auto img_len = 4 * h * (w + 1);
struct idat {
  uint8_t cmf = 0x78;
  uint8_t flg = 0x01;
  uint8_t bhead = 1; // No compression, final block
  uint16_t len = yoyo::flip16(img_len);
  uint16_t nlen = yoyo::flip16(~img_len);
  uint8_t pixels[img_len]{};
  uint32_t adler{};
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

// TODO: support IDAT split at 16k
int main() {
  idat i{};
  auto *sl = i.pixels;
  for (auto y = 0; y < h; y++) {
    *sl++ = 0; // filter 0
    for (auto x = 0; x < w; x++, sl += 4) {
      sl[0] = sl[3] = 0xFF;
      sl[1] = 256 * x / w;
      sl[2] = 256 * y / h;
    }
  }
  i.adler = yoyo::flip32(adler(i.pixels, sizeof(i.pixels)));

  return yoyo::file_writer::open("out/test.png")
      .fmap(frk::signature("PNG"))
      .fmap(frk::chunk("IHDR", ihdr{w, h}))
      .fmap(frk::chunk("IDAT", i))
      .fmap(frk::chunk("IEND"))
      .map(frk::end())
      .map([] { return 0; })
      .log_error([] { return 1; });
}
