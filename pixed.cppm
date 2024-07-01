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

mno::req<void> read_ihdr(yoyo::reader &r, dec_ctx &img);
static constexpr auto read_ihdr(dec_ctx &img) {
  return [&](auto &r) { return read_ihdr(r, img); };
}

mno::req<void> read_idat(yoyo::reader &r, dec_ctx &img);
static constexpr auto read_idat(dec_ctx &img) {
  return [&](auto &r) { return read_idat(r, img); };
}

export mno::req<dec_ctx> read(const char *file) {
  dec_ctx res{};
  return yoyo::file_reader::open(file)
      .fpeek(frk::assert("PNG"))
      .fpeek(read_ihdr(res))
      .fpeek(read_idat(res))
      .fpeek(frk::take("IEND"))
      .map(frk::end())
      .map([&] { return traits::move(res); });
}
} // namespace pixed
