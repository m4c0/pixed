module atlased;
import buoy;
import fork;
import silog;

static constexpr const auto max_name_len = 1024;
static struct {
  char name[max_name_len]{};
} g_cur_file;

void atlased::config::load() {
  buoy::open_for_reading("pixed", "atlased")
      .fpeek(frk::assert("PXD"))
      .fpeek(frk::take("fILE", &g_cur_file))
      .map(frk::end())
      .take(silog::log_failure);
}

static void write() {
  buoy::open_for_writing("pixed", "atlased")
      .fpeek(frk::signature("PXD"))
      .fpeek(frk::chunk("fILE", g_cur_file))
      .map(frk::end())
      .take(silog::log_failure);
}

void atlased::config::set_current_file(jute::view name) {
  auto &f = g_cur_file.name;
  unsigned n{};
  for (auto c : name) {
    if (n >= max_name_len)
      return;

    f[n++] = c;
  }

  write();
}
