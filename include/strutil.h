/*
 * keyd - A key remapping daemon.
 *
 * © 2019 Raheman Vaiya (see also: LICENSE).
 */
#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stdlib.h>

int utf8_read_char(const char *_s, uint32_t *code);
int utf8_strlen(const char *s);
char* nexttoken(char** str, char delim);

int is_timeval(const char *s);
size_t str_escape(char *s);
#endif
