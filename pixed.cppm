export module pixed;
import fork;
import hai;
import missingno;
import traits;
import yoyo;

using namespace traits::ints;

namespace pixed {
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

export constexpr auto idat(const void *img, unsigned w, unsigned h) {
  return [=](yoyo::writer &wr) -> mno::req<void> {
    auto buf = filter(static_cast<const uint8_t *>(img), w, h);
    return spliterate_idat(wr, buf.begin(), buf.size());
  };
}
} // namespace pixed
