#pragma leco tool

import flate;
import fork;
import hai;
import silog;
import stubby;
import traits;
import yoyo;

using namespace traits::ints;

static constexpr auto ihdr(int &w, int &h) {
  return [&](yoyo::subreader r) {
    return r.read_u32_be()
        .map([&](auto n) { w = n; })
        .fmap([&] { return r.read_u32_be(); })
        .map([&](auto n) { h = n; });
    // TODO: assert format, etc
  };
}

static constexpr auto run_filter(hai::array<uint8_t> &data, int filter,
                                 unsigned y) {
  switch (filter) {
  case 0:
    return mno::req<void>{};
  case 1:
    // TODO: implement
    return mno::req<void>{};
  case 2:
    // TODO: implement
    if (y == 0)
      return mno::req<void>::failed("'up' filter at top row");

    return mno::req<void>{};
  case 3:
    return mno::req<void>::failed("average filter not supported");
  case 4:
    return mno::req<void>::failed("paeth filter not supported");
  default:
    return mno::req<void>::failed("unsupported filter");
  }
}

static constexpr auto deflate(const int &w, const int &h) {
  return [&](yoyo::subreader r) {
    hai::array<uint8_t> data{static_cast<unsigned>(w * h * 4)};
    flate::bitstream b{&r};
    return r.read_u16()
        .assert([](auto cmf_flg) { return cmf_flg == 0x0178; },
                "only 32k window deflate is supported")
        .fmap([&](auto) { return flate::huffman_reader::create(&b); })
        .fmap([&](auto &hr) {
          mno::req<void> res{};
          for (auto y = 0; y < h && res.is_valid(); y++) {
            res = hr.read_u8().fmap([&](auto filter) {
              mno::req<void> res{};
              for (auto x = 0; x < w * 4 && res.is_valid(); x++) {
                res = hr.read_u8().map([](auto) {});
              }
              return res.fmap([&] { return run_filter(data, filter, y); });
            });
          }
          return res;
        })
        .map([&] {
          auto d = reinterpret_cast<stbi::pixel *>(data.begin());
          stbi::write_rgba_unsafe("out/test.png", w, h, d);
        });
  };
}

int main() {
  int w;
  int h;
  yoyo::file_reader::open("blank.png")
      .fmap(frk::assert("PNG"))
      .fmap(frk::take("IHDR", ihdr(w, h)))
      .fmap(frk::take("IDAT", deflate(w, h)))
      .fmap(frk::take("IEND"))
      .map(frk::end())
      .log_error([] { throw 0; });
}
