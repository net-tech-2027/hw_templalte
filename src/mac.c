#include "mac.h"
/*
 * Заглушка (skeleton): логики нет. Реализуйте контракт из include/mac.h.
 * Пока реализация не написана, тесты будут падать — это ожидаемо.
 */
int mac_serialize(const mac_addr_t *mac, char *out, size_t out_size) {
  (void)mac;
  (void)out;
  (void)out_size;
  return -1;
}

int mac_deserialize(const char *text, mac_addr_t *mac) {
  (void)text;
  (void)mac;
  return -1;
}
