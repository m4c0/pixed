#pragma leco tool

import hai;
import pixed;
import traits;
import yoyo;

using namespace traits::ints;

static mno::req<void> scale(const pixed::context &ctx, unsigned sz,
                            hai::varray<uint8_t> *buf) {
  auto icon = pixed::create(sz, sz);
  buf->set_capacity(102400);

  auto *p = icon.image.begin();
  for (auto y = 0; y < sz; y++) {
    for (auto x = 0; x < sz; x++, p++) {
      *p = {255, 255, 255, 255};
    }
  }

  return pixed::write(*buf, icon);
}

static mno::req<void> convert_to_ico(const pixed::context &ctx) {
  if (ctx.w != 1024 || ctx.h != 1024)
    return mno::req<void>::failed("expecting 1024x1024 source icon");

  constexpr const auto num_imgs = 3;

  auto res = yoyo::file_writer::open("icon.ico")
                 .fpeek(yoyo::write_u16(0))         // Reserved
                 .fpeek(yoyo::write_u16(1))         // Type: icon
                 .fpeek(yoyo::write_u16(num_imgs)); // Number of images
  if (!res.is_valid())
    return res.map([](auto &) {}).trace("writing ICO header");

  constexpr const auto hdr_sz = 6;
  constexpr const auto img_hdr_sz = 16;

  constexpr const unsigned sizes[num_imgs]{64, 32, 16};
  hai::varray<uint8_t> bufs[num_imgs]{};
  unsigned ofs = hdr_sz + img_hdr_sz * num_imgs;
  auto *buf = bufs;
  for (auto s : sizes) {
    auto r = scale(ctx, s, buf);
    if (!r.is_valid())
      return r.trace("writing downscaled image");

    auto sz = buf->size();

    res = res.fpeek(yoyo::write_u8(s))      // Width
              .fpeek(yoyo::write_u8(s))     // Height
              .fpeek(yoyo::write_u8(0))     // Num. of colours
              .fpeek(yoyo::write_u8(0))     // Reserved
              .fpeek(yoyo::write_u16(0))    // Colour planes
              .fpeek(yoyo::write_u16(32))   // Bits per pixel
              .fpeek(yoyo::write_u32(sz))   // Image size
              .fpeek(yoyo::write_u32(ofs)); // Image offset
    if (!res.is_valid())
      return res.map([](auto &) {}).trace("writing image subheader");

    ofs += sz;
    buf++;
  }

  for (auto &buf : bufs) {
    res = res.fpeek(yoyo::write(buf.begin(), buf.size()));
    if (!res.is_valid())
      return res.map([](auto &) {}).trace("writing image data");
  }

  return res.map([](auto &) {});
}

int main(int argc, char **argv) try {
  convert_to_ico(pixed::create(1024, 1024)).log_error();
  // pixed::read("icon.png").fmap(convert_to_ico).log_error();

  return 0;
} catch (...) {
  return 1;
}
