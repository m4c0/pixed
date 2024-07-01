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
export struct context {
  unsigned w;
  unsigned h;
  hai::varray<uint8_t> compress{};
  hai::array<uint8_t> image{};
};
export context create(unsigned w, unsigned h) {
  return {.w = w, .h = h, .image{w * h * 4}};
}

export mno::req<void> write(const char *file, context &img);
export mno::req<context> read(const char *file);
} // namespace pixed
