#pragma leco tool
import gopt;
import fork;
import hai;
import jute;
import pixed;
import silog;
import traits;
import yoyo;

using namespace traits::ints;

static void usage() {
  silog::log(silog::error, R"(
Usage: pnger.exe -i <input> (-r | -n | -a <pal> | ...) [-o <output>]

Where:
        -a: append <pal> (RRGGBBAA) to sPLT
        -i: input filename
        -n: creates or clear palette in sPLT
        -o: output filename. If absent, no file modification is done
)");
  throw 0;
}

void append_palette(pixed::context &ctx, const char *val) {
  uint32_t n{};
  auto c = val;
  for (auto i = 0; i < 4; i++) {
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
  }

  auto &pal = ctx.palette;
  pal.set_capacity(pal.size() + 1);
  pal[pal.size() - 1] = *reinterpret_cast<pixed::pixel *>(&n);
}

int main(int argc, char **argv) try {
  pixed::context ctx{};

  auto opts = gopt_parse(argc, argv, "i:o:rna:", [&](auto ch, auto val) {
    switch (ch) {
    case 'a':
      append_palette(ctx, val);
      break;
    case 'n':
      ctx.palette = {};
      break;

    case 'i':
      ctx = pixed::read(val)
                .trace("reading palette from input file")
                .log_error([] { throw 0; });
      break;
    case 'o':
      pixed::write(val, ctx).trace("writing output file").log_error([] {
        throw 0;
      });
      break;
    default:
      usage();
      break;
    }
  });
  if (opts.argc != 0)
    usage();

  if (ctx.palette.size() == 0) {
    silog::log(silog::info, "No palette present");
  } else {
    unsigned i{};
    unsigned count = ctx.palette.size();
    silog::log(silog::info, "Palette size: %d", count);
    for (auto p : ctx.palette) {
      silog::log(silog::info, "- Colour %3d: %02x%02x%02x%02x", ++i, p.r, p.g,
                 p.b, p.a);
    }
  }

  return 0;
} catch (...) {
  return 1;
}
