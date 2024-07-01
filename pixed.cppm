#pragma leco add_impl read
#pragma leco add_impl write

export module pixed;
import fork;
import hai;
import missingno;
import traits;
import yoyo;

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
  hai::array<pixel> image{};
};

export context create(unsigned w, unsigned h) {
  return {.w = w, .h = h, .image{w * h * 4}};
}

export mno::req<void> write(const char *file, context &img);
export mno::req<context> read(const char *file);
} // namespace pixed
