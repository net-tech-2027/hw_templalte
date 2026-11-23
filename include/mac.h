#ifndef NET_COURSE_MAC_H
#define NET_COURSE_MAC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Длина MAC-адреса в октетах (48 бит). */
#define MAC_ADDR_LEN 6

/* Длина канонической строки без завершающего '\0': "AA:BB:CC:DD:EE:FF". */
#define MAC_ADDR_STR_LEN 17

/* Размер буфера под каноническую строку с учётом завершающего '\0'. */
#define MAC_ADDR_STR_SIZE (MAC_ADDR_STR_LEN + 1)

/*
 * MAC-адрес устройства — последовательность из MAC_ADDR_LEN октетов.
 * octets[0] — старший октет (как в записи "AA:BB:CC:DD:EE:FF").
 */
typedef struct mac_addr {
  uint8_t octets[MAC_ADDR_LEN];
} mac_addr_t;

/*
 * Сериализует MAC-адрес в каноническую строку "AA:BB:CC:DD:EE:FF":
 * ровно MAC_ADDR_STR_LEN символов, hex-цифры в верхнем регистре,
 * разделитель — двоеточие, в конце '\0'.
 *
 * out_size — размер буфера out в байтах (включая место под '\0').
 *
 * Возвращает MAC_ADDR_STR_LEN или -1, если:
 *   - mac == NULL или out == NULL;
 *   - out_size < MAC_ADDR_STR_SIZE.
 *
 * При ошибке содержимое out не изменяется.
 *
 * TODO(student): реализуйте в src/mac.c. Изменять этот заголовок нельзя.
 */
int mac_serialize(const mac_addr_t *mac, char *out, size_t out_size);

/*
 * Десериализует MAC-адрес из строки. Принимается только канонический вид:
 * ровно MAC_ADDR_STR_LEN символов "hh:hh:hh:hh:hh:hh", где h — hex-цифра;
 * регистр не важен ('a'..'f' и 'A'..'F'). Лишние пробелы и другие
 * разделители не допускаются.
 *
 * Возвращает 0 при успехе или -1, если:
 *   - text == NULL или mac == NULL;
 *   - длина text не равна MAC_ADDR_STR_LEN;
 *   - ':' отсутствует на позициях 2, 5, 8, 11, 14;
 *   - на позициях hex-цифр находятся другие символы.
 *
 * При ошибке содержимое *mac не изменяется.
 *
 * TODO(student): реализуйте в src/mac.c. Изменять этот заголовок нельзя.
 */
int mac_deserialize(const char *text, mac_addr_t *mac);

#ifdef __cplusplus
}
#endif

#endif /* NET_COURSE_MAC_H */
