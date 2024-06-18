#pragma leco tool

import hai;
import traits;
import stubby;

using namespace traits::ints;

using pixel = stbi::pixel;

class image {
  unsigned m_width{};
  unsigned m_height{};
  hai::array<pixel> m_data{};

public:
  constexpr image() = default;
  image(unsigned w, unsigned h) : m_width{w}, m_height{h}, m_data{w * h} {}

  pixel at(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
      return {};
    return m_data[y * m_width + x];
  }
  pixel &at(int x, int y) {
    static pixel dummy{};
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
      return dummy;

    return m_data[y * m_width + x];
  }

  void write(const char *fname) const {
    stbi::write_rgba(fname, m_width, m_height, m_data);
  }
};

int main() {
  pixel pal[]{
      {0, 0, 0, 0},
      {255, 255, 255, 255},
  };
  image img{32, 16};
  img.at(16, 8) = pal[0];
  img.at(16, 9) = pal[1];
  img.write("out/test.png");
}
