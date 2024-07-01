module pixed;
import flate;
import fork;
import silog;

using namespace pixed;

static mno::req<void> read_ihdr(yoyo::reader &r, unsigned &w, unsigned &h) {
  return r.read_u32_be()
      .map([&](auto n) { w = n; })
      .fmap([&] { return r.read_u32_be(); })
      .map([&](auto n) { h = n; })
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
      .map([&](auto) { silog::log(silog::debug, "found %dx%d image", w, h); });
}
static constexpr auto read_ihdr(unsigned &w, unsigned &h) {
  return [&](yoyo::subreader r) { return read_ihdr(r, w, h); };
}
static mno::req<void> read_ihdr(yoyo::reader &r, context &ctx) {
  return frk::take("IHDR", ::read_ihdr(ctx.w, ctx.h))(r);
}
static constexpr auto read_ihdr(context &img) {
  return [&](auto &r) { return read_ihdr(r, img); };
}

static constexpr auto run_filter(void *d, int filter, unsigned y, int w) {
  auto data = static_cast<uint8_t *>(d);
  switch (filter) {
  case 0:
    return mno::req<void>{};
  case 1:
    for (auto x = 4; x < w * 4; x++) {
      data[x] += data[x - 4];
    }
    return mno::req<void>{};
  case 2:
    if (y == 0)
      return mno::req<void>{};

    for (auto x = 0; x < w * 4; x++) {
      data[x] += data[x - w * 4];
    }
    return mno::req<void>{};
  case 3:
    if (y == 0)
      return mno::req<void>{};

    for (auto x = 0; x < 4; x++) {
      data[x] = data[x - w * 4];
    }
    for (auto x = 4; x < w * 4; x++) {
      data[x] += (data[x - 4] + data[x - w * 4]) / 2;
    }
    return mno::req<void>{};
  case 4:
    return mno::req<void>::failed("paeth filter not supported");
  default:
    return mno::req<void>::failed("unsupported filter");
  }
}

static constexpr auto deflate(context &img) {
  return [&] {
    img.image = hai::array<pixed::pixel>{img.h * img.w};

    yoyo::memreader r{img.compress.begin(), img.compress.size()};
    flate::bitstream b{&r};
    return r.read_u16()
        .assert([](auto id) { return id == 0x0178; },
                "only 32k window deflate is supported")
        .fmap([&](auto) { return flate::huffman_reader::create(&b); })
        .fmap([&](auto &hr) {
          mno::req<void> res{};
          for (auto y = 0; y < img.h && res.is_valid(); y++) {
            res = hr.read_u8().fmap([&](auto filter) {
              void *ptr = img.image.begin() + y * img.w;
              return hr.read(ptr, img.w * 4)
                  .fmap([&] { return run_filter(ptr, filter, y, img.w); })
                  .trace("reading scanline");
            });
          }
          return res;
        });
  };
}

static constexpr auto idat(hai::varray<uint8_t> &data) {
  return [&](yoyo::subreader r) {
    auto size = data.size() + r.raw_size();
    data.add_capacity(size);
    return r.read(data.end(), r.raw_size()).map([&] { data.expand(size); });
  };
}

static mno::req<void> read_idat(yoyo::reader &r, context &img) {
  return frk::take_all("IDAT", idat(img.compress))(r).fmap(deflate(img));
}
static constexpr auto read_idat(context &img) {
  return [&](auto &r) { return read_idat(r, img); };
}

mno::req<context> pixed::read(const char *file) {
  context res{};
  return yoyo::file_reader::open(file)
      .fpeek(frk::assert("PNG"))
      .fpeek(read_ihdr(res))
      .fpeek(read_idat(res))
      .fpeek(frk::take("IEND"))
      .map(frk::end())
      .map([&] { return traits::move(res); });
}
