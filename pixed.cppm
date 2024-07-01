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
export struct dec_ctx {
  unsigned w;
  unsigned h;
  hai::varray<uint8_t> compress{};
  hai::array<uint8_t> image{};
};
export mno::req<void> write(const char *file, dec_ctx &img);
export mno::req<dec_ctx> read(const char *file);
} // namespace pixed
