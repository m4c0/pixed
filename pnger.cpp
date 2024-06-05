#pragma leco tool
import gopt;
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

void usage() {
  silog::log(silog::error, R"(
Usage: pnger.exe [-r | -n | -a <pal>]

Where:
        -r: remove palette from sPLT
        -n: creates or clear palette in sPLT
        -a: append <pal> (RRGGBBAA) to sPLT

Notes:
        "-r", "-n" and "-a" are mutually exclusive. If neither is informed, dump sPLT.
)");
  throw 0;
}

int main(int argc, char **argv) try {
  const char *pal{};
  char mode{};
  auto opts = gopt_parse(argc, argv, "rna:", [&](auto ch, auto val) {
    switch (ch) {
    case 'a':
      pal = val;
    // fallthrough
    case 'r':
    case 'n':
      if (mode)
        usage();
      mode = ch;
      break;
    default:
      usage();
      break;
    }
  });
  if (opts.argc != 0)
    usage();

  bool res = yoyo::file_reader::open("blank.png")
                 .fmap(dump_png)
                 .map([] { return true; })
                 .log_error();

  return res ? 0 : 1;
} catch (...) {
  return 1;
}
