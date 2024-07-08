#pragma leco app

import casein;
import quack;
import voo;

static void atlas(voo::h2l_image *img) {}

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
    max_quads(10240);

    clear_colour({0.1f, 0.15f, 0.1f, 1.f});
    push_constants({
        .grid_pos = {0.5f, 0.5f},
        .grid_size = {1, 1},
    });
    atlas([](auto dq) {
      return new voo::updater<voo::h2l_image>{
          dq->queue(), atlas, dq->physical_device(), 256U, 256U};
    });
    data(::data);
  }
} i;
