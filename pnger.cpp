#pragma leco tool
import gopt;
import hai;
import jute;
import silog;
import traits;
import yoyo;

using namespace traits::ints;
using chunk_t = hai::array<uint8_t>;

static bool signature_matches(uint64_t hdr) {
  return hdr == 0x0A1A0A0D474E5089;
}

static mno::req<chunk_t> read_sPLT(yoyo::reader &in) {
  uint32_t type{};
  chunk_t data{};
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
        auto name =
            jute::view::unsafe(reinterpret_cast<const char *>(data.begin()));
        if (type == 'TLPs' && name == "pixed palette")
          return traits::move(data);

        return chunk_t{};
      });
}

static mno::req<chunk_t> find_sPLT_in_png(yoyo::reader &in) {
  return in.read_u64()
      .assert(signature_matches, "file signature doesn't match")
      .map([](auto _signature) { return chunk_t{}; })
      .until_failure([&](auto &&res) { return read_sPLT(in); },
                     [&](auto msg) { return !in.eof().unwrap(false); });
}

void usage() {
  silog::log(silog::error, R"(
Usage: pnger.exe -i <input> (-r | -n | -a <pal> | ...) [-o <output>]

Where:
        -a: append <pal> (RRGGBBAA) to sPLT
        -i: input filename
        -n: creates or clear palette in sPLT
        -o: output filename. If absent, no file modification is done
        -r: remove palette from sPLT
)");
  throw 0;
}

int main(int argc, char **argv) try {
  chunk_t sPLT{};
  auto opts = gopt_parse(argc, argv, "i:o:rna:", [&](auto ch, auto val) {
    switch (ch) {
    case 'a':
    case 'r':
    case 'n':
      break;

    case 'i':
      sPLT = yoyo::file_reader::open(val).fmap(find_sPLT_in_png).log_error([] {
        throw 0;
      });
      break;
    case 'o':
      break;
    default:
      usage();
      break;
    }
  });
  if (opts.argc != 0)
    usage();

  return 0;
} catch (...) {
  return 1;
}
