#pragma leco tool
import gopt;
import hai;
import jute;
import silog;
import traits;
import yoyo;

using namespace traits::ints;
using chunk_t = hai::varray<uint8_t>;

static constexpr const auto pal_name = jute::view{"pixed palette"};
static constexpr const unsigned initial_size = sizeof(pal_name) + 1;

static bool signature_matches(uint64_t hdr) {
  return hdr == 0x0A1A0A0D474E5089;
}

static mno::req<chunk_t> read_sPLT(yoyo::reader &in) {
  uint32_t type{};
  chunk_t data{};
  return in.read_u32_be()
      .fmap([&](auto l) {
        data.set_capacity(l);
        data.expand(l);
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
        if (type == 'TLPs' && name == pal_name)
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

chunk_t new_sPLT() {
  chunk_t res{initial_size};
  res.expand(initial_size);

  char *buf = reinterpret_cast<char *>(res.begin());
  for (auto c : pal_name)
    *buf++ = c;

  *buf++ = 8;
  return res;
}

void append_sPLT(chunk_t &sPLT, const char *val) {
  if (sPLT.size() == 0) {
    silog::log(silog::error, "attempt of appending to a non-existing palette");
    throw 0;
  }

  auto c = val;
  for (auto p = 0; p < 4; p++) {
    uint8_t n{};
    for (auto n = 0; n < 2; n++, c++) {
      n <<= 4;
      if (*c >= '0' && *c <= '9') {
        n |= (*c - '0');
      } else if (*c >= 'A' && *c <= 'F') {
        n |= (*c - 'A');
      } else if (*c >= 'a' && *c <= 'f') {
        n |= (*c - 'a');
      } else {
        silog::log(silog::error, "invalid colour definition [%s]", val);
        throw 0;
      }
    }
    sPLT.push_back_doubling(n);
  }

  // Frequency - zero'd since this is a pseudo-palette
  sPLT.push_back_doubling(0);
  sPLT.push_back_doubling(0);
}

int main(int argc, char **argv) try {
  chunk_t sPLT{};
  auto opts = gopt_parse(argc, argv, "i:o:rna:", [&](auto ch, auto val) {
    switch (ch) {
    case 'a':
      append_sPLT(sPLT, val);
      break;
    case 'r':
      sPLT = {};
      break;
    case 'n':
      sPLT = new_sPLT();
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

  if (sPLT.size() == 0) {
    silog::log(silog::info, "No palette present");
  } else {
    unsigned size = sPLT.size() - initial_size;
    unsigned count = size / 6;
    silog::log(silog::info, "Palette size: %d", count);
    auto c = sPLT.begin() + initial_size;
    for (auto i = 0; i < count; i++, c += 6) {
      silog::log(silog::info, "- Colour %3d: %02x%02x%02x%02x", i + 1, c[0],
                 c[1], c[2], c[3]);
    }
  }

  return 0;
} catch (...) {
  return 1;
}
