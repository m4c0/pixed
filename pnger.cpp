#pragma leco tool

import traits;
import yoyo;

using namespace traits::ints;

bool signature_matches(uint64_t hdr) { return hdr == 0x0A1A0A0D474E5089; }

mno::req<void> dump_png(yoyo::reader &in) {
  return in.read_u64()
      .assert(signature_matches, "file signature doesn't match")
      .map([](auto) {});
}

int main() {
  bool res = yoyo::file_reader::open("blank.png")
                 .fmap(dump_png)
                 .map([] { return true; })
                 .log_error();

  return res ? 0 : 1;
}
