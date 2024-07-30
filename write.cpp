module pixed;
import flate;
import fork;
import hai;
import jute;
import missingno;
import silog;
import sitime;
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

  ihdr() = default;
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

static auto compress(const hai::array<uint8_t> &data) {
  constexpr const auto part_size = 1 << 14;
  constexpr const auto blk_size = 5 + part_size;

  const auto blks = (data.size() + part_size - 1) / part_size;
  hai::array<uint8_t> out{2 + (blks * blk_size) + 4};

  auto res = mno::req{yoyo::memwriter{out}}
                 .fpeek(yoyo::write_u8(0x78)) // CMF
                 .fpeek(yoyo::write_u8(0x1))  // FLG
                 .fpeek(flate::compress(data.begin(), data.size()));

  return res.fpeek(yoyo::write_u32_be(adler(data.begin(), data.size())))
      .map([&](auto &) { return traits::move(out); });
}

static mno::req<void> write_idat(yoyo::writer &wr, const void *img, unsigned w,
                                 unsigned h) {
  auto buf = filter(static_cast<const uint8_t *>(img), w, h);
  return compress(buf).fmap([&](auto &buf) {
    return frk::chunk("IDAT", buf.begin(), buf.size())(wr);
  });
}

static constexpr auto write_idat(const void *img, unsigned w, unsigned h) {
  return [=](auto &wr) { return write_idat(wr, img, w, h); };
}

static constexpr auto write_splt(const hai::varray<pixed::pixel> &pal) {
  return [&](auto &wr) {
    if (pal.size() == 0)
      return mno::req<void>{};

    constexpr const jute::view name = "PIXED";
    constexpr const unsigned name_size = name.size();
    hai::array<uint8_t> data{name_size + 2 + 6 * pal.size()};
    auto res = mno::req{yoyo::memwriter{data}}
                   .fpeek(yoyo::write(name.data(), name_size))
                   .fpeek(yoyo::write_u8(0))  // null terminator
                   .fpeek(yoyo::write_u8(8)); // sample depth
    for (auto p : pal) {
      res = res.fpeek(yoyo::write(&p, sizeof(p)))
                .fpeek(yoyo::write_u16(0)); // Frequency
    }
    return res.fmap([&](auto &) {
      return frk::chunk("sPLT", data.begin(), data.size())(wr);
    });
  };
}

mno::req<void> pixed::write(hai::varray<uint8_t> &buf, context &img) {
  return mno::req{yoyo::memwriter{buf}}
      .fpeek(frk::signature("PNG"))
      .fpeek(write_ihdr(img.w, img.h))
      .fpeek(write_splt(img.palette))
      .fpeek(frk::chunk("spSZ", img.spr_size))
      .fpeek(write_idat(img.image.begin(), img.w, img.h))
      .fpeek(frk::chunk("IEND"))
      .map([&](auto &w) { buf.expand(w.raw_pos()); });
}
mno::req<void> pixed::write(const char *file, context &img) {
  sitime::stopwatch w{};
  return yoyo::file_writer::open(file)
      .fpeek(frk::signature("PNG"))
      .fpeek(write_ihdr(img.w, img.h))
      .fpeek(write_splt(img.palette))
      .fpeek(frk::chunk("spSZ", img.spr_size))
      .fpeek(write_idat(img.image.begin(), img.w, img.h))
      .fpeek(frk::chunk("IEND"))
      .map(frk::end())
      .map([&] {
        silog::log(silog::debug, "Image created in %dms", w.millis());
      });
}
