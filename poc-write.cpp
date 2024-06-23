#pragma leco tool
import fork;
import traits;
import yoyo;

using namespace traits::ints;

#pragma pack(push, 1)
struct ihdr {
  uint32_t width = yoyo::flip32(16);
  uint32_t height = yoyo::flip32(16);
  uint8_t bit_depth = 8;
  uint8_t colour_type = 6; // RGBA
  uint8_t compression = 0; // Deflate
  uint8_t filter = 0;
  uint8_t interlace = 0;
};
static constexpr const auto img_len = 4 * 16 * (16 + 1);
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

int main() {
  idat i{};
  i.adler = yoyo::flip32(adler(i.pixels, sizeof(i.pixels)));

  return yoyo::file_writer::open("out/test.png")
      .fmap(frk::signature("PNG"))
      .fmap(frk::chunk("IHDR", ihdr{}))
      .fmap(frk::chunk("IDAT", i))
      .fmap(frk::chunk("IEND"))
      .map(frk::end())
      .map([] { return 0; })
      .log_error([] { return 1; });
}
