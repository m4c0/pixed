#pragma leco tool
import gopt;
import hai;
import jute;
import silog;
import traits;
import yoyo;

using namespace traits::ints;
using chunk_data_t = hai::varray<uint8_t>;

struct chunk {
  uint32_t type;
  chunk_data_t data;
  uint32_t crc;
};

static void usage() {
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

static constexpr const auto pal_name = jute::view{"pixed palette"};
static constexpr const unsigned initial_size = sizeof(pal_name) + 1;

static bool signature_matches(uint64_t hdr) {
  return hdr == 0x0A1A0A0D474E5089;
}

static mno::req<chunk> read_chunk(yoyo::reader &in) {
  chunk res{};
  return in.read_u32_be()
      .fmap([&](auto l) {
        res.data.set_capacity(l);
        res.data.expand(l);
        return in.read_u32();
      })
      .fmap([&](auto t) {
        res.type = t;
        if (res.data.size() == 0)
          return mno::req<void>{};

        return in.read(res.data.begin(), res.data.size());
      })
      .fmap([&] { return in.read_u32(); })
      .map([&](auto crc) {
        res.crc = crc;
        return traits::move(res);
      });
}

static bool is_sPLT(const chunk &c) {
  auto &[type, data, crc] = c;
  auto name = jute::view::unsafe(reinterpret_cast<const char *>(data.begin()));
  return type == 'TLPs' && name == pal_name;
}

static mno::req<chunk_data_t> read_sPLT(chunk_data_t current,
                                        yoyo::reader &in) {
  return read_chunk(in).map(
      [&](chunk &c) { return traits::move(is_sPLT(c) ? c.data : current); });
}

static mno::req<chunk_data_t> find_sPLT_in_png(yoyo::reader &in) {
  return in.read_u64()
      .assert(signature_matches, "file signature doesn't match")
      .map([](auto _signature) { return chunk_data_t{}; })
      .until_failure(
          [&](auto &&res) { return read_sPLT(traits::move(res), in); },
          [&](auto msg) { return !in.eof().unwrap(false); });
}

chunk_data_t new_sPLT() {
  chunk_data_t res{initial_size};
  res.expand(initial_size);

  char *buf = reinterpret_cast<char *>(res.begin());
  for (auto c : pal_name)
    *buf++ = c;

  *buf++ = 8;
  return res;
}

void append_sPLT(chunk_data_t &sPLT, const char *val) {
  if (sPLT.size() == 0) {
    silog::log(silog::error, "attempt of appending to a non-existing palette");
    throw 0;
  }

  auto c = val;
  for (auto i = 0; i < 4; i++) {
    uint8_t n{};
    for (auto j = 0; j < 2; j++, c++) {
      n <<= 4;
      if (*c >= '0' && *c <= '9') {
        n |= (*c - '0');
      } else if (*c >= 'A' && *c <= 'F') {
        n |= (*c - 'A') + 10;
      } else if (*c >= 'a' && *c <= 'f') {
        n |= (*c - 'a') + 10;
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

static mno::req<void> write_chunk(yoyo::writer &out, uint32_t type,
                                  const chunk_data_t &data) {
  return mno::req<void>{};
}

static mno::req<void> pass_chunk(yoyo::reader &in, yoyo::writer &out,
                                 const chunk_data_t &sPLT) {
  return read_chunk(in).fmap([&](chunk &c) {
    if (is_sPLT(c))
      return mno::req<void>{};

    if (c.type == 'TADI' && sPLT.size() > 0)
      return write_chunk(out, 'TLPs', sPLT).fmap([&] {
        return write_chunk(out, c.type, c.data);
      });

    return write_chunk(out, c.type, c.data);
  });
}
static mno::req<void> replace_sPLT(yoyo::reader &in, yoyo::writer &out,
                                   const chunk_data_t &sPLT) {
  return in.read_u64()
      .assert(signature_matches, "file signature doesn't match")
      .fmap([&](auto signature) { return out.write(signature); })
      .until_failure([&] { return pass_chunk(in, out, sPLT); },
                     [&](auto msg) { return !in.eof().unwrap(false); });
}

int main(int argc, char **argv) try {
  mno::req<yoyo::file_reader> in{};
  chunk_data_t sPLT{};
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
      in = yoyo::file_reader::open(val);
      sPLT = in.fmap(find_sPLT_in_png).log_error([] { throw 0; });
      break;
    case 'o': {
      in.fmap([](auto &in) { return in.seekg(0, yoyo::seek_mode::set); })
          .fmap([&] { return yoyo::file_writer::open(val); })
          .fmap([&](auto &out) {
            return in.fmap(
                [&](auto &in) { return replace_sPLT(in, out, sPLT); });
          })
          .log_error([] { throw 0; });
      break;
    }
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
