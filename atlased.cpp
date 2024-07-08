#pragma leco app

import casein;
import quack;
import voo;

static quack::donald::atlas_t *atlas(voo::device_and_queue *dq) {
  return new voo::sires_image{"atlas.png", dq};
}

static unsigned data(quack::instance *i) {
  i->colour = {1, 1, 1, 1};
  i->multiplier = {1, 1, 1, 1};
  i->position = {0, 0};
  i->size = {1, 1};
  i->uv0 = {0, 0};
  i->uv1 = {1, 1};
  i->rotation = {};
  return 1;
}

struct init {
  init() {
    using namespace quack::donald;

    app_name("atlas-editor");
    max_quads(16);

    clear_colour({0.1f, 0.15f, 0.1f, 1.f});
    push_constants({
        .grid_pos = {0.5f, 0.5f},
        .grid_size = {1, 1},
    });
    atlas(::atlas);
    data(::data);
  }
} i;
