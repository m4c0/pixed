module atlased;
import buoy;
import fork;
import hai;
import silog;
import traits;
import yoyo;

static hai::cstr g_cur_file{};

static auto read_cstr(yoyo::subreader sr) {
  g_cur_file = hai::cstr{static_cast<unsigned>(sr.raw_size())};
  return sr.read(g_cur_file.begin(), g_cur_file.size());
}
static auto read_image() {
  return pixed::read(g_cur_file.begin())
      .map([](auto &ctx) {
        g_ctx = traits::move(ctx);
        if (g_ctx.spr_size.x == 0 || g_ctx.spr_size.y == 0) {
          g_ctx.spr_size = dotz::ivec2{8};
        }
        silog::log(silog::info, "Loaded [%s] based on config",
                   g_cur_file.begin());
      })
      .trace("loading image from config file");
}

void atlased::config::load() {
  buoy::open_for_reading("pixed", "atlased")
      .fpeek(frk::assert("PXD"))
      .fpeek(frk::take("fILE", read_cstr))
      .map(frk::end())
      .fmap(read_image)
      .trace("loading atlased config")
      .take(silog::log_failure);
}

static void write() {
  buoy::open_for_writing("pixed", "atlased")
      .fpeek(frk::signature("PXD"))
      .fpeek(frk::chunk("fILE", g_cur_file.begin(), g_cur_file.size()))
      .map(frk::end())
      .trace("writing atlased config")
      .take(silog::log_failure);
}

void atlased::config::set_current_file(jute::view name) {
  g_cur_file = name.cstr();
  write();
}
