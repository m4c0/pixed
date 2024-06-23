#pragma leco tool

import flate;
import fork;
import hai;
import silog;
import stubby;
import traits;
import yoyo;

using namespace traits::ints;

struct png {
  unsigned w;
  unsigned h;
  hai::varray<uint8_t> data{};
};

static constexpr auto ihdr(unsigned &w, unsigned &h) {
  return [&](yoyo::subreader r) {
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
        .map(
            [&](auto) { silog::log(silog::debug, "found %dx%d image", w, h); });
  };
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

static constexpr auto deflate(const png &img) {
  return [&] {
    if (img.data[0] != 0x78 && img.data[1] != 0x01)
      return mno::req<void>::failed("only 32k window deflate is supported");

    hai::array<stbi::pixel> data{img.w * img.h};
    yoyo::memreader r{img.data.begin() + 2, img.data.size() - 2};
    flate::bitstream b{&r};
    return flate::huffman_reader::create(&b)
        .fmap([&](auto &hr) {
          mno::req<void> res{};
          for (auto y = 0; y < img.h && res.is_valid(); y++) {
            res = hr.read_u8().fmap([&](auto filter) {
              void *ptr = data.begin() + y * img.w;
              return hr.read(ptr, img.w * 4)
                  .fmap([&] { return run_filter(ptr, filter, y, img.w); })
                  .trace("reading scanline");
            });
          }
          return res;
        })
        .map([&] {
          auto d = reinterpret_cast<stbi::pixel *>(data.begin());
          stbi::write_rgba_unsafe("out/test.png", img.w, img.h, d);
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

int main() {
  png img{};
  yoyo::file_reader::open("blank.png")
      .fmap(frk::assert("PNG"))
      .fmap(frk::take("IHDR", ihdr(img.w, img.h)))
      .fmap(frk::take_all("IDAT", idat(img.data)))
      .fmap(frk::take("IEND"))
      .map(frk::end())
      .fmap(deflate(img))
      .log_error([] { throw 0; });
}
