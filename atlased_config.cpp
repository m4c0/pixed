module atlased;
import buoy;
import fork;
import silog;

static struct {
  char name[1024]{};
} g_cur_file;

void atlased::config::load() {
  buoy::open_for_reading("pixed", "atlased")
      .fpeek(frk::assert("PXD"))
      .fpeek(frk::take("fILE", &g_cur_file))
      .map(frk::end())
      .take(silog::log_failure);
}
