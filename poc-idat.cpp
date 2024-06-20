#pragma leco tool

import flate;
import fork;
import silog;
import traits;
import yoyo;

mno::req<void> ihdr(yoyo::subreader r) {
  return r.read_u32_be()
      .map([](auto w) { silog::log(silog::debug, "image w = %d", w); })
      .fmap([&] { return r.read_u32_be(); })
      .map([](auto w) { silog::log(silog::debug, "image h = %d", w); });
}

mno::req<void> deflate(yoyo::subreader r) {
  unsigned count{};

  r.seekg(2).take([](auto err) { throw 0; });

  flate::bitstream b{&r};
  return flate::huffman_reader::create(&b)
      .until_failure(
          [&](auto &hr) {
            return hr.read_u8().map([&](auto r) {
              count++;
              return traits::move(hr);
            });
          },
          [](auto) { return false; })
      .map([&](auto &) { silog::log(silog::debug, "read %d", count); })
      .fmap([&] { return r.tellg(); })
      .map([](unsigned h) { silog::log(silog::debug, "tellg %d", h); });
}

int main() {
  yoyo::file_reader::open("blank.png")
      .fmap(frk::assert("PNG"))
      .fmap(frk::take("IHDR", ihdr))
      .fmap(frk::take("IDAT", deflate))
      .fmap(frk::take("IEND"))
      .map(frk::end())
      .log_error([] { throw 0; });
}
