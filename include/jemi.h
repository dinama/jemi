#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct jemi_s {
  void* cb_baton;
  void (*cb_write)(char const*, size_t, void*);
  unsigned int nwrited;
  int pretty;

  char b[512];
  unsigned nb;

  unsigned ndeep;
  char deep[256];
  unsigned count[256];
};

void jemi_init_ex(struct jemi_s*, void (*)(char const*, size_t, void*), void*);
void jemi_init(struct jemi_s*);
struct jemi_s* jemi_flush(struct jemi_s*);
struct jemi_s* jemi_write(struct jemi_s*, char const*, size_t);
struct jemi_s* jemi_raw(struct jemi_s*, char const*, size_t);
struct jemi_s* jemi_raw_c(struct jemi_s*, char const*);
struct jemi_s* jemi_nindent(struct jemi_s*, size_t);
struct jemi_s* jemi_indent(struct jemi_s*);
struct jemi_s* jemi_json(struct jemi_s*, char const*, size_t);
struct jemi_s* jemi_json_c(struct jemi_s*, char const*);
struct jemi_s* jemi_null(struct jemi_s*);
struct jemi_s* jemi_quote(struct jemi_s*);
struct jemi_s* jemi_plain(struct jemi_s*, char const*, size_t);
struct jemi_s* jemi_plain_c(struct jemi_s*, char const*);
struct jemi_s* jemi_encode(struct jemi_s*, char const*, size_t);
struct jemi_s* jemi_encode_c(struct jemi_s*, char const*);
struct jemi_s* jemi_integer(struct jemi_s*, int64_t);
struct jemi_s* jemi_uinteger(struct jemi_s*, uint64_t);
struct jemi_s* jemi_real(struct jemi_s*, double);
struct jemi_s* jemi_boolean(struct jemi_s*, int);

struct jemi_s* jemi_object(struct jemi_s*);
struct jemi_s* jemi_map_name_beg(struct jemi_s* j);
struct jemi_s* jemi_map_name_end(struct jemi_s* j);
struct jemi_s* jemi_map(struct jemi_s*, char const*);
struct jemi_s* jemi_array(struct jemi_s*);
struct jemi_s* jemi_value(struct jemi_s*);
struct jemi_s* jemi_pop(struct jemi_s*);
struct jemi_s* jemi_done(struct jemi_s*);

#ifdef __cplusplus
}
#endif

