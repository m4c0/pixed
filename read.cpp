module pixed;
import flate;
import fork;
import jute;
import yoyo;
import silog;

using namespace pixed;

static mno::req<void> read_ihdr(yoyo::reader &r, unsigned &w, unsigned &h,
                                unsigned &ct) {
  return r.read_u32_be()
      .map([&](auto n) { w = n; })
      .fmap([&] { return r.read_u32_be(); })
      .map([&](auto n) { h = n; })
      .fmap([&] { return r.read_u8(); })
      .assert([](auto bit_depth) { return bit_depth == 8; },
              "unsupported bitdepth")
      .fmap([&](auto) { return r.read_u8(); })
      .assert(
          [](auto colour_type) { return colour_type == 6 || colour_type == 2; },
          "unsupported colour type")
      .map([&](auto n) { ct = (n == 6) ? 4 : 3; })
      .fmap([&] { return r.read_u8(); })
      .assert([](auto compression) { return compression == 0; },
              "unsupported compression")
      .fmap([&](auto) { return r.read_u8(); })
      .assert([](auto filter) { return filter == 0; }, "unsupported filter")
      .fmap([&](auto) { return r.read_u8(); })
      .assert([](auto interlace) { return interlace == 0; },
              "unsupported interlace")
      .map([&](auto) { silog::log(silog::debug, "found %dx%d image", w, h); });
}
static constexpr auto read_ihdr(unsigned &w, unsigned &h, unsigned &ct) {
  return [&](yoyo::subreader r) { return read_ihdr(r, w, h, ct); };
}
static mno::req<void> read_ihdr(yoyo::reader &r, context &ctx) {
  return frk::take("IHDR", ::read_ihdr(ctx.w, ctx.h, ctx.ct))(r);
}
static constexpr auto read_ihdr(context &img) {
  return [&](auto &r) { return read_ihdr(r, img); };
}

static constexpr int paeth_pred(int a, int b, int c) {
  // a = left, b = above, c = upper-left
  int p = a + b - c;
  int pa = dotz::abs(p - a);
  int pb = dotz::abs(p - b);
  int pc = dotz::abs(p - c);
  if (pa <= pb && pa <= pc)
    return a;
  if (pb <= pc)
    return b;
  return c;
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
    if (y == 0) {
      for (auto x = 4; x < w * 4; x++) {
        data[x] += data[x - 4] / 2;
      }
      return mno::req<void>{};
    }

    for (auto x = 0; x < 4; x++) {
      data[x] += data[x - w * 4] / 2;
    }
    for (auto x = 4; x < w * 4; x++) {
      data[x] += (data[x - 4] + data[x - w * 4]) / 2;
    }
    return mno::req<void>{};
  case 4:
    if (y == 0)
      return mno::req<void>{};

    for (auto x = 0; x < w * 4; x++) {
      int a = x >= 4 ? data[x - 4] : 0;
      int b = y >= 1 ? data[x - w * 4] : 0;
      int c = x >= 4 && y >= 1 ? data[x - w * 4 - 4] : 0;
      data[x] += paeth_pred(a, b, c);
    }
    return mno::req<void>{};
  default:
    return mno::req<void>::failed("unsupported filter");
  }
}

static constexpr auto deflate_4ct_scanline(context &img, yoyo::reader &hr) {
  mno::req<void> res{};
  for (auto y = 0; y < img.h && res.is_valid(); y++) {
    res = hr.read_u8().fmap([&](auto filter) {
      void *ptr = img.image.begin() + y * img.w;
      return hr.read(ptr, img.w * 4)
          .fmap([&] { return run_filter(ptr, filter, y, img.w); })
          .trace("reading RGBA scanline");
    });
  }
  return res;
}

static constexpr auto deflate_3ct_scanline(context &img, yoyo::reader &hr) {
  mno::req<void> res{};
  for (auto y = 0; y < img.h && res.is_valid(); y++) {
    unsigned filter{};
    res = hr.read_u8().map([&](auto f) { filter = f; });
    for (auto x = 0; x < img.w && res.is_valid(); x++) {
      void *ptr = img.image.begin() + y * img.w + x;
      res = hr.read(ptr, 3);
    }
    void *ptr = img.image.begin() + y * img.w;
    res = run_filter(ptr, filter, y, img.w);
    for (auto x = 0; x < img.w; x++) {
      img.image[y * img.w + x].a = 255;
    }
  }
  return res.trace("reading RGB scanlines");
}

static constexpr auto deflate(context &img, hai::varray<uint8_t> &zlib) {
  return [&] {
    img.image = hai::array<pixed::pixel>{img.h * img.w};

    yoyo::memreader r{zlib.begin(), zlib.size()};
    flate::bitstream b{&r};
    return r.read_u16()
        .assert([](auto id) { return id & 0x0008; },
                "only deflate is supported")
        .fmap([&](auto) { return flate::huffman_reader::create(&b); })
        .fmap([&](auto &hr) {
          if (img.ct == 4)
            return deflate_4ct_scanline(img, hr);
          return deflate_3ct_scanline(img, hr);
        });
  };
}

static constexpr auto idat(hai::varray<uint8_t> &data) {
  return [&](yoyo::subreader r) {
    auto size = data.size() + r.raw_size();
    data.set_capacity(size);
    return r.read(data.end(), r.raw_size()).map([&] { data.expand(size); });
  };
}

static mno::req<void> read_idat(yoyo::reader &r, context &img) {
  auto good_size_guess = img.w * img.h * img.ct * 2;
  hai::varray<uint8_t> zlib{good_size_guess};
  return frk::take_all("IDAT", idat(zlib))(r).fmap(deflate(img, zlib));
}
static constexpr auto read_idat(context &img) {
  return [&](auto &r) { return read_idat(r, img); };
}

static constexpr auto read_splt(context &img) {
  return [&](auto &r) {
    hai::array<char> buf{static_cast<unsigned>(r.raw_size())};
    return r.read(buf.begin(), buf.size()).map([&] {
      if (jute::view{"PIXED"} != jute::view::unsafe(buf.begin()))
        return;
      if (buf[6] != 8) // sample depth
        return;

      unsigned pal_size = buf.size() - 7;
      if (pal_size % 6 != 0)
        return;

      img.palette = hai::varray<pixed::pixel>{pal_size / 6U};
      img.palette.expand(img.palette.capacity());
      auto pb = buf.begin() + 7;
      for (auto &p : img.palette) {
        p = *reinterpret_cast<pixed::pixel *>(pb);
        pb += 6;
      }
    });
  };
}

mno::req<context> pixed::read(const char *file) {
  context res{};
  return yoyo::file_reader::open(file)
      .fpeek(frk::assert("PNG"))
      .fpeek(read_ihdr(res))
      .fpeek(frk::take("sPLT", read_splt(res)))
      .fpeek(frk::take("spSZ", &res.spr_size))
      .fpeek(read_idat(res))
      .fpeek(frk::take("IEND"))
      .map(frk::end())
      .map([&] { return traits::move(res); });
}
