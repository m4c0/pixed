module pixed;
import fork;
import hai;
import missingno;
import traits;
import yoyo;

using namespace traits::ints;

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

static mno::req<void> write_ihdr(yoyo::writer &wr, unsigned w, unsigned h) {
  return frk::chunk("IHDR", ihdr{w, h})(wr);
}
static constexpr auto write_ihdr(unsigned w, unsigned h) {
  return [=](auto &wr) { return write_ihdr(wr, w, h); };
}

static constexpr uint32_t adler(const uint8_t *data, unsigned len) {
#if 1
  // Looks like no one cares about this value
  return 0;
#else
  static constexpr const auto base = 65521;
  uint32_t s1 = 1;
  uint32_t s2 = 0;
  for (auto i = 0; i < len; i++) {
    s1 = (s1 + data[i]) % base;
    s2 = (s2 + s1) % base;
  }
  return (s2 << 16) + s1;
#endif
}

static constexpr auto min(auto a, auto b) { return a > b ? b : a; }
static auto spliterate_idat(auto &png, const uint8_t *img, unsigned size) {
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

static mno::req<void> write_idat(yoyo::writer &wr, const void *img, unsigned w,
                                 unsigned h) {
  auto buf = filter(static_cast<const uint8_t *>(img), w, h);
  return spliterate_idat(wr, buf.begin(), buf.size());
}

static constexpr auto write_idat(const void *img, unsigned w, unsigned h) {
  return [=](auto &wr) { return write_idat(wr, img, w, h); };
}

mno::req<void> pixed::write(const char *file, context &img) {
  return yoyo::file_writer::open(file)
      .fpeek(frk::signature("PNG"))
      .fpeek(write_ihdr(img.w, img.h))
      .fpeek(write_idat(img.image.begin(), img.w, img.h))
      .fpeek(frk::chunk("IEND"))
      .map(frk::end());
}
