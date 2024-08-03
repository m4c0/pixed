module atlased;
import silog;
import traits;

static dotz::ivec2 g_mark{};
static dotz::ivec2 g_cursor{};
static dotz::ivec2 g_yank{};
static bool g_saving{};

dotz::ivec2 atlased::mark() { return g_mark; }

static unsigned data(quack::instance *start) {
  auto i = start;
  auto [sw, sh] = atlased::grid_size();
  dotz::vec2 sz = atlased::grid_size();

  quack::donald::push_constants({
      .grid_pos = sz / 2.0f,
      .grid_size = sz,
  });

  if (!g_saving)
    *i++ = {
        .position = g_cursor,
        .size = {1, 1},
        .colour = {1, 0, 0, 1},
    };

  for (dotz::vec2 p{}; p.y < sh; p.y++) {
    for (p.x = 0; p.x < sw; p.x++) {
      *i++ = {
          .position = p + 0.05f,
          .size = dotz::vec2(0.9f),
          .uv0 = g_mark / sz,
          .uv1 = (g_mark + 1) / sz,
          .multiplier = {1, 1, 1, 1},
      };
      *i++ = {
          .position = p + 0.05f,
          .size = dotz::vec2(0.9f),
          .uv0 = p / sz,
          .uv1 = (p + 1) / sz,
          .multiplier = {1, 1, 1, 1},
      };
    }
  }
  return i - start;
}

static void cursor(dotz::ivec2 d) {
  auto gs = atlased::grid_size();
  g_cursor = (g_cursor + d + gs) % gs;
  quack::donald::data(::data);
}

static void spr_size(dotz::ivec2 d) {
  do {
    g_ctx.spr_size = g_ctx.spr_size + d;
  } while (dotz::sq_length(atlased::image_size() % g_ctx.spr_size) != 0);

  g_cursor = dotz::min(g_cursor, atlased::grid_size() - 1);
  quack::donald::data(::data);
}

static void files_drop() {
  for (auto &file : casein::dropped_files) {
    atlased::config::set_current_file(file)
        .map([] { quack::donald::data(::data); })
        .log_error();
  }
}

static void put() {
  for (dotz::ivec2 p{}; p.y < g_ctx.spr_size.y; p.y++) {
    for (p.x = 0; p.x < g_ctx.spr_size.x; p.x++) {
      auto yank = pixed::at(g_ctx, g_yank * g_ctx.spr_size + p);
      auto &cursor = pixed::at(g_ctx, g_cursor * g_ctx.spr_size + p);
      cursor = yank;
    }
  }
  atlased::load_atlas();
}

static void read() {
  auto fn = atlased::config::current_file();
  atlased::config::set_current_file(jute::view::unsafe(fn)).log_error();
  quack::donald::data(::data);
}
static void write() {
  const char *fn = atlased::config::current_file();
  if (!fn || !*fn)
    fn = "atlas.png";

  g_saving = 1;
  quack::donald::data(::data);
  pixed::write(fn, g_ctx)
      .map([] {
        silog::log(silog::info, "Image written successfully");
        g_saving = 0;
        quack::donald::data(::data);
      })
      .log_error();
}

static void (*g_arrow_fn)(dotz::ivec2 d);

void atlased::modes::atlas() {
  using namespace casein;

  g_arrow_fn = cursor;

  reset_casein();

  handle(FILES_DROP, files_drop);

  handle(KEY_DOWN, K_DOT, [] { g_arrow_fn = spr_size; });
  handle(KEY_UP, K_DOT, [] { g_arrow_fn = cursor; });

  handle(KEY_DOWN, K_E, read);
  handle(KEY_DOWN, K_W, write);

  handle(KEY_DOWN, K_M, [] {
    g_mark = g_cursor;
    quack::donald::data(::data);
  });

  handle(KEY_DOWN, K_Y, [] { g_yank = g_cursor; });
  handle(KEY_DOWN, K_P, put);

  handle(KEY_DOWN, K_LEFT, [] { g_arrow_fn({-1, 0}); });
  handle(KEY_DOWN, K_RIGHT, [] { g_arrow_fn({1, 0}); });
  handle(KEY_DOWN, K_UP, [] { g_arrow_fn({0, -1}); });
  handle(KEY_DOWN, K_DOWN, [] { g_arrow_fn({0, 1}); });
  handle(KEY_DOWN, K_ENTER, [] { modes::sprite(g_cursor); });

  quack::donald::data(::data);
}
