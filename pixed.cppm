#pragma leco add_impl read
#pragma leco add_impl write

export module pixed;
import dotz;
import hai;
import missingno;
import traits;

using namespace traits::ints;

namespace pixed {
export struct pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};
static_assert(sizeof(pixel) == 4);

export struct context {
  unsigned w;
  unsigned h;
  dotz::ivec2 spr_size{};
  hai::varray<pixel> palette{};
  hai::array<pixel> image{};
};

export [[nodiscard]] context create(unsigned w, unsigned h) {
  return {.w = w, .h = h, .image{w * h}};
}

export [[nodiscard]] mno::req<void> write(hai::array<uint8_t> &buf,
                                          context &img);
export [[nodiscard]] mno::req<void> write(const char *file, context &img);
export [[nodiscard]] mno::req<context> read(const char *file);

export pixel &at(context &ctx, int x, int y) {
  static pixel dummy{};

  if (x < 0 || x >= ctx.w || y < 0 || y >= ctx.h)
    return dummy;

  return ctx.image[y * ctx.w + x];
}
export pixel &at(context &ctx, dotz::ivec2 p) { return at(ctx, p.x, p.y); }
} // namespace pixed
