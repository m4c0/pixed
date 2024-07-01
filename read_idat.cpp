module pixed;
import flate;
import fork;

using namespace pixed;

static constexpr auto run_filter(void *d, int filter, unsigned y, int w) {
  auto data = static_cast<uint8_t *>(d);
  switch (filter) {
  case 0:
    return mno::req<void>{};
  case 1:
    for (auto x = 4; x < w * 4; x++) {
      data[x] += data[x - 4];
    }
    return mno::req<void>{};
  case 2:
    if (y == 0)
      return mno::req<void>{};

    for (auto x = 0; x < w * 4; x++) {
      data[x] += data[x - w * 4];
    }
    return mno::req<void>{};
  case 3:
    if (y == 0)
      return mno::req<void>{};

    for (auto x = 0; x < 4; x++) {
      data[x] = data[x - w * 4];
    }
    for (auto x = 4; x < w * 4; x++) {
      data[x] += (data[x - 4] + data[x - w * 4]) / 2;
    }
    return mno::req<void>{};
  case 4:
    return mno::req<void>::failed("paeth filter not supported");
  default:
    return mno::req<void>::failed("unsupported filter");
  }
}

static constexpr auto deflate(dec_ctx &img) {
  return [&] {
    img.image = hai::array<uint8_t>{img.h * img.w * 4};

    yoyo::memreader r{img.compress.begin(), img.compress.size()};
    flate::bitstream b{&r};
    return r.read_u16()
        .assert([](auto id) { return id == 0x0178; },
                "only 32k window deflate is supported")
        .fmap([&](auto) { return flate::huffman_reader::create(&b); })
        .fmap([&](auto &hr) {
          mno::req<void> res{};
          for (auto y = 0; y < img.h && res.is_valid(); y++) {
            res = hr.read_u8().fmap([&](auto filter) {
              void *ptr = img.image.begin() + y * img.w * 4;
              return hr.read(ptr, img.w * 4)
                  .fmap([&] { return run_filter(ptr, filter, y, img.w); })
                  .trace("reading scanline");
            });
          }
          return res;
        });
  };
}

static constexpr auto idat(hai::varray<uint8_t> &data) {
  return [&](yoyo::subreader r) {
    auto size = data.size() + r.raw_size();
    data.add_capacity(size);
    return r.read(data.end(), r.raw_size()).map([&] { data.expand(size); });
  };
}

mno::req<void> pixed::read_idat(yoyo::reader &r, dec_ctx &img) {
  return frk::take_all("IDAT", idat(img.compress))(r).fmap(deflate(img));
}
