#pragma leco tool

import flate;
import fork;
import silog;
import traits;
import yoyo;

static constexpr auto ihdr(int &w, int &h) {
  return [&](yoyo::subreader r) {
    return r.read_u32_be()
        .map([&](auto n) { w = n; })
        .fmap([&] { return r.read_u32_be(); })
        .map([&](auto n) { h = n; });
    // TODO: assert format, etc
  };
}

static constexpr auto deflate(const int &w, const int &h) {
  return [&](yoyo::subreader r) {
    flate::bitstream b{&r};
    return r.read_u16()
        .assert([](auto cmf_flg) { return cmf_flg == 0x0178; },
                "only 32k window deflate is supported")
        .fmap([&](auto) { return flate::huffman_reader::create(&b); })
        .fmap([&](auto &hr) {
          mno::req<void> res{};
          for (auto y = 0; y < h && res.is_valid(); y++) {
            res = hr.read_u8().fmap([&](auto filter) {
              mno::req<void> res =
                  filter <= 2 ? mno::req<void>{}
                              : mno::req<void>::failed("unsupported filter");
              for (auto x = 0; x < w * 4 && res.is_valid(); x++) {
                res = hr.read_u8().map([](auto) {});
              }
              return res;
            });
          }
          return res;
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
