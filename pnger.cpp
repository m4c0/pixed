#pragma leco tool

import yoyo;

int main() {
  return yoyo::file_reader::open("blank.png")
      .map([](auto &) { return 0; })
      .unwrap(1);
}
