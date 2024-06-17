#pragma leco tool
import gopt;
import fork;
import hai;
import jute;
import silog;
import traits;
import yoyo;

using namespace traits::ints;
using chunk_data_t = hai::varray<char>;

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
static constexpr const unsigned initial_size = pal_name.size() + 2;

chunk_data_t new_sPLT() {
  chunk_data_t res{initial_size};
  res.expand(initial_size);

  char *buf = res.begin();
  for (auto c : pal_name)
    *buf++ = c;

  *buf++ = 0;
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

int main(int argc, char **argv) try {
  const char *input{};
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
      input = val;
      yoyo::file_reader::open(val)
          .fmap(frk::assert("PNG"))
          .fmap(frk::take("IHDR"))
          .fmap(frk::take("sPLT",
                          [&](yoyo::subreader r) {
                            chunk_data_t data{
                                static_cast<unsigned>(r.raw_size())};
                            data.set_capacity(r.raw_size());
                            data.expand(r.raw_size());
                            return r.read(data.begin(), data.size()).map([&] {
                              if (jute::view::unsafe(data.begin()) != pal_name)
                                return;

                              sPLT = traits::move(data);
                            });
                          }))
          .fmap(frk::take("IDAT"))
          .fmap(frk::take("IEND"))
          .map(frk::end())
          .trace("reading palette from input file")
          .log_error([] { throw 0; });
      break;
    case 'o':
      frk::copy::start("PNG", val)
          .fmap([&] {
            return yoyo::file_reader::open(input)
                .fmap(frk::assert("PNG"))
                .fmap(frk::copy::chunk("IHDR", val))
                .fmap([&](auto &&r) {
                  return yoyo::file_writer::append(val)
                      .fmap(frk::chunk("sPLT", sPLT.begin(), sPLT.size()))
                      .map(frk::end())
                      .map([&] { return traits::move(r); });
                })
                .fmap(frk::copy::chunk("IDAT", val))
                .fmap(frk::copy::chunk("IEND", val))
                .map(frk::end())
                .trace("writing output file");
          })
          .trace("copying chunks from input file")
          .log_error([] { throw 0; });
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
      silog::log(silog::info, "- Colour %3d: %02x%02x%02x%02x", i + 1,
                 c[0] & 0xFF, c[1] & 0xFF, c[2] & 0xFF, c[3] & 0xFF);
    }
  }

  return 0;
} catch (...) {
  return 1;
}
