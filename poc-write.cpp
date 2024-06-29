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

static constexpr auto adler(const uint8_t *data, unsigned len) {
  static constexpr const auto base = 65521;
  uint32_t s1 = 1;
  uint32_t s2 = 0;
  for (auto i = 0; i < len; i++) {
    s1 = (s1 + data[i]) % base;
    s2 = (s2 + s1) % base;
  }
  return (s2 << 16) + s1;
}

static constexpr auto min(auto a, auto b) { return a > b ? b : a; }
static constexpr auto spliterate_idat(const uint8_t *img, unsigned size) {
  return [=](auto &png) mutable {
    constexpr const auto buf_size = 1 << 14;

    hai::array<uint8_t> buf{buf_size};
    auto res = mno::req{yoyo::memwriter{buf}}
                   .fpeek(yoyo::write_u8(0x78)) // CMF
                   .fpeek(yoyo::write_u8(0x1)); // FLG
    while (res.is_valid() && size > 0) {
      // TODO: really compress

      unsigned len{};
      unsigned bhead{};
      res = res.peek([&](auto &w) {
                 len = min(w.raw_size() - w.raw_pos() - 5, size);
                 bhead = len == size;
               })
                .fpeek(yoyo::write_u8(bhead))
                .fpeek(yoyo::write_u16(len))
                .fpeek(yoyo::write_u16(~len))
                .fpeek(yoyo::write(img, len))
                .fpeek(yoyo::seek(0, yoyo::seek_mode::set))
                .peek([&](auto &w) {
                  img += len;
                  size -= len;
                })
                .fpeek([&](auto &w) {
                  return size == 0 ? w.write_u32_be(adler(img, size))
                                   : mno::req<void>{};
                })
                .fpeek([&](auto &w) {
                  return frk::chunk("IDAT", buf.begin(), w.raw_size())(png);
                });
    }
    return res.map([](auto &) {});
  };
}

static hai::array<uint8_t> filter(const uint8_t *img, unsigned w, unsigned h) {
  hai::array<uint8_t> res{(w * 4 + 1) * h};
  auto *sl = res.begin();
  for (auto y = 0; y < h; y++) {
    // TODO: other filters
    *sl++ = 0; // filter 0
    for (auto x = 0; x < w * 4; x++) {
      *sl++ = *img++;
    }
  }
  return res;
}

int main() {
  static constexpr const auto w = 1024;
  static constexpr const auto h = 1024;
  static constexpr const auto img_len = 4 * h * w;

  hai::array<uint8_t> img{img_len};
  auto *sl = img.begin();
  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++, sl += 4) {
      sl[0] = sl[3] = 0xFF;
      sl[1] = 256 * x / w;
      sl[2] = 256 * y / h;
    }
  }

  auto buf = filter(img.begin(), w, h);

  return yoyo::file_writer::open("out/test.png")
      .fpeek(frk::signature("PNG"))
      .fpeek(frk::chunk("IHDR", ihdr{w, h}))
      .fpeek(spliterate_idat(buf.begin(), buf.size()))
      .fpeek(frk::chunk("IEND"))
      .map(frk::end())
      .map([] { return 0; })
      .log_error([] { return 1; });
}
