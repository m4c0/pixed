#pragma leco add_impl idat
export module pixed;
import missingno;
import traits;
import yoyo;

using namespace traits::ints;

namespace pixed {
mno::req<void> idat(yoyo::writer &wr, const void *img, unsigned w, unsigned h);
export constexpr auto idat(const void *img, unsigned w, unsigned h) {
  return [=](auto &wr) { return idat(wr, img, w, h); };
}
} // namespace pixed
