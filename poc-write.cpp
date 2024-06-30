#pragma leco tool
import fork;
import hai;
import pixed;
import traits;
import yoyo;

using namespace traits::ints;

#pragma pack(push, 1)
struct ihdr {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth = 8;
  uint8_t colour_type = 6; // RGBA
  uint8_t compression = 0; // Deflate
  uint8_t filter = 0;
  uint8_t interlace = 0;

  ihdr(uint32_t w, uint32_t h)
      : width{yoyo::flip32(w)}
      , height{yoyo::flip32(h)} {}
};
#pragma pack(pop)
static_assert(sizeof(ihdr) == 13);

int main() {
  static constexpr const auto w = 1024;
  static constexpr const auto h = 1024;
  static constexpr const auto img_len = 4 * h * w;

  hai::array<uint8_t> img{img_len};
  auto *sl = img.begin();
  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++, sl += 4) {
      sl[0] = sl[3] = 0xFF;
      sl[1] = 256 * x / w;
      sl[2] = 256 * y / h;
    }
  }

  return yoyo::file_writer::open("out/test.png")
      .fpeek(frk::signature("PNG"))
      .fpeek(frk::chunk("IHDR", ihdr{w, h}))
      .fpeek(pixed::write_idat(img.begin(), w, h))
      .fpeek(frk::chunk("IEND"))
      .map(frk::end())
      .map([] { return 0; })
      .log_error([] { return 1; });
}
