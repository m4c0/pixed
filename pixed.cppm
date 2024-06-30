#pragma leco add_impl write_idat
export module pixed;
import missingno;
import yoyo;

namespace pixed {
mno::req<void> write_idat(yoyo::writer &wr, const void *img, unsigned w,
                          unsigned h);
export constexpr auto write_idat(const void *img, unsigned w, unsigned h) {
  return [=](auto &wr) { return write_idat(wr, img, w, h); };
}
} // namespace pixed
