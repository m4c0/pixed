#pragma leco add_impl ihdr
#pragma leco add_impl read_idat
#pragma leco add_impl write_idat

export module pixed;
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

mno::req<void> write_ihdr(yoyo::writer &wr, unsigned w, unsigned h);
export constexpr auto write_ihdr(unsigned w, unsigned h) {
  return [=](auto &wr) { return write_ihdr(wr, w, h); };
}

mno::req<void> write_idat(yoyo::writer &wr, const void *img, unsigned w,
                          unsigned h);
export constexpr auto write_idat(const void *img, unsigned w, unsigned h) {
  return [=](auto &wr) { return write_idat(wr, img, w, h); };
}

mno::req<void> read_ihdr(yoyo::reader &r, dec_ctx &img);
export constexpr auto read_ihdr(dec_ctx &img) {
  return [&](auto &r) { return read_ihdr(r, img); };
}

mno::req<void> read_idat(yoyo::reader &r, dec_ctx &img);
export constexpr auto read_idat(dec_ctx &img) {
  return [&](auto &r) { return read_idat(r, img); };
}

} // namespace pixed
