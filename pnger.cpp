#pragma leco tool

import hai;
import silog;
import traits;
import yoyo;

using namespace traits::ints;

static bool signature_matches(uint64_t hdr) {
  return hdr == 0x0A1A0A0D474E5089;
}

static mno::req<void> dump_chunk(yoyo::reader &in) {
  uint32_t type{};
  hai::array<uint8_t> data{};
  return in.read_u32_be()
      .fmap([&](auto l) {
        data.set_capacity(l);
        return in.read_u32();
      })
      .fmap([&](auto t) {
        type = t;
        if (data.size() == 0)
          return mno::req<void>{};

        return in.read(data.begin(), data.size());
      })
      .fmap([&] { return in.read_u32(); })
      .map([&](auto crc) {
        silog::log(silog::info, "%.*s %d", 4,
                   reinterpret_cast<const char *>(&type), data.size());
      });
}

static mno::req<void> dump_png(yoyo::reader &in) {
  return in.read_u64()
      .assert(signature_matches, "file signature doesn't match")
      .map([](auto) {})
      .until_failure([&] { return dump_chunk(in); },
                     [&](auto msg) { return !in.eof().unwrap(false); });
}

int main() {
  bool res = yoyo::file_reader::open("blank.png")
                 .fmap(dump_png)
                 .map([] { return true; })
                 .log_error();

  return res ? 0 : 1;
}
