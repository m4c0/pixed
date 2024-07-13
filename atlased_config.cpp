module atlased;
import buoy;
import fork;
import hai;
import silog;
import traits;
import yoyo;

static hai::cstr g_cur_file{};

static auto read_image(jute::view file) {
  auto cstr = file.cstr();
  return pixed::read(cstr.begin())
      .map([&](auto &ctx) {
        if (ctx.spr_size.x == 0 || ctx.spr_size.y == 0)
          ctx.spr_size = {16, 16};

        g_ctx = traits::move(ctx);

        g_cur_file = traits::move(cstr);

        auto [sw, sh] = atlased::grid_size();
        silog::log(silog::info, "Number of sprites: %dx%d (total: %d)", sw, sh,
                   sw * sh);

        atlased::load_atlas();

        silog::log(silog::info, "Image loaded [%s]", g_cur_file.begin());
      })
      .trace("loading image from config file");
}
static auto read_curfile(yoyo::subreader sr) {
  auto buf = hai::cstr{static_cast<unsigned>(sr.raw_size())};
  return sr.read(buf.begin(), buf.size()).fmap([&] { return read_image(buf); });
}

void atlased::config::load() {
  buoy::open_for_reading("pixed", "atlased")
      .fpeek(frk::assert("PXD"))
      .fpeek(frk::take("fILE", read_curfile))
      .map(frk::end())
      .trace("loading atlased config")
      .take(silog::log_failure);
}

static void write_config() {
  buoy::open_for_writing("pixed", "atlased")
      .fpeek(frk::signature("PXD"))
      .fpeek(frk::chunk("fILE", g_cur_file.begin(), g_cur_file.size()))
      .map(frk::end())
      .trace("writing atlased config")
      .take(silog::log_failure);
}

const char *atlased::config::current_file() { return g_cur_file.begin(); }
mno::req<void> atlased::config::set_current_file(jute::view name) {
  return read_image(name).map(write_config);
}
