#pragma leco tool

import hai;
import pixed;
import traits;
import yoyo;

using namespace traits::ints;

static mno::req<void> convert_to_ico(const pixed::context &ctx) {
  constexpr const auto w = 16;
  constexpr const auto h = 16;

  constexpr const auto hdr_sz = 6;
  constexpr const auto img_hdr_sz = 16;
  constexpr const auto ofs = hdr_sz + img_hdr_sz;

  auto icon = pixed::create(16, 16);
  hai::array<uint8_t> buf{102400};
  unsigned size{};
  auto res = pixed::write(buf, icon)
                 .map([&](auto s) { size = s; })
                 .fmap([] { return yoyo::file_writer::open("icon.ico"); })
                 .fpeek(yoyo::write_u16(0)) // Reserved
                 .fpeek(yoyo::write_u16(1)) // Type: icon
                 .fpeek(yoyo::write_u16(1)) // Number of images

                 .fpeek(yoyo::write_u8(w))     // Width
                 .fpeek(yoyo::write_u8(h))     // Height
                 .fpeek(yoyo::write_u8(0))     // Num. of colours
                 .fpeek(yoyo::write_u8(0))     // Reserved
                 .fpeek(yoyo::write_u16(0))    // Colour planes
                 .fpeek(yoyo::write_u16(32))   // Bits per pixel
                 .fpeek(yoyo::write_u32(size)) // Image size
                 .fpeek(yoyo::write_u32(ofs)); // Image offset

  res = res.fpeek(yoyo::write(buf.begin(), size));

  return res.map([](auto &) {});
}

int main(int argc, char **argv) try {
  convert_to_ico(pixed::create(1024, 1024)).log_error();
  // pixed::read("icon.png").fmap(convert_to_ico).log_error();

  return 0;
} catch (...) {
  return 1;
}
