#include "jemi.h"
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>


static void jemi_fwrite(char const* s, size_t ns, void* baton)
{
  fwrite(s, ns, 1, (FILE*)baton);
}

void jemi_init_ex(struct jemi_s* j, void (*cb)(char const*, size_t, void*), void* b)
{
  memset(j, 0, sizeof(*j));
  j->cb_baton = b;
  j->cb_write = cb;
}

void jemi_init(struct jemi_s* j)
{
  jemi_init_ex(j, jemi_fwrite, stdout);
}

struct jemi_s* jemi_flush(struct jemi_s* j)
{
  if (j->nb) {
    jemi_write(j, j->b, j->nb);
    j->nb = 0;
  }
  if (jemi_fwrite == j->cb_write)
    fflush((FILE*)j->cb_baton);

  return j;
}

struct jemi_s* jemi_write(struct jemi_s* j, char const* s, size_t ns)
{
  j->nwrited += ns;
  if (j->cb_write) j->cb_write(s, ns, j->cb_baton);
  return j;
}

struct jemi_s* jemi_raw(struct jemi_s* j, char const* s, size_t ns)
{
  char const* e  = s + ns;
  while (s < e) {
    if (!(j->nb < sizeof(j->b)))
      jemi_flush(j);
    j->b[j->nb++] = *s++;
  }
  return j;
}

struct jemi_s* jemi_raw_c(struct jemi_s* j, char const* s)
{
  while (s && *s) {    
    if (!(j->nb < sizeof(j->b)))
      jemi_flush(j);
    j->b[j->nb++] = *s++;
  }
  return j;
}

struct jemi_s* jemi_nindent(struct jemi_s* j, size_t sz)
{
  while (sz --> 0) jemi_raw_c(j, "  ");
  return j;
}

struct jemi_s* jemi_indent(struct jemi_s* j)
{
  if (j->pretty && j->ndeep) return jemi_nindent(j, j->ndeep);
  return j;
}

struct jemi_s* jemi_json(struct jemi_s* j, char const* s, size_t ns)
{

  if (!j->pretty)
    return jemi_raw(j, s, ns);

  char const* p  = s;
  char const* e  = s + ns;
  while (s < e) {
    if ('\n' == *s) {
      jemi_raw(j, p, s - p);
      jemi_indent(jemi_raw_c(j, "\n"));
      p = ++s;
    } else s++;
  }
  return jemi_raw(j, p, s - p);
}

struct jemi_s* jemi_json_c(struct jemi_s* j, char const* s)
{

  if (!j->pretty)
    return jemi_raw_c(j, s);

  char const* p  = s;
  while (s && *s) {
    if ('\n' == *s) {
      jemi_raw(j, p, s - p);
      jemi_indent(jemi_raw_c(j, "\n"));
      p = ++s;
    } else s++;
  }
  return jemi_raw(j, p, s - p);
}

struct jemi_s* jemi_null(struct jemi_s* j)
{
  return jemi_raw_c(j, "null");
}

struct jemi_s* jemi_quote(struct jemi_s* j)
{
  return jemi_raw_c(j, "\"");
}

struct jemi_s* jemi_plain(struct jemi_s* j, char const* s, size_t ns)
{
  return jemi_quote(jemi_raw(jemi_quote(j), s, ns));
}

struct jemi_s* jemi_plain_c(struct jemi_s* j, char const* s)
{
  return jemi_quote(jemi_raw_c(jemi_quote(j), s));
}

struct jemi_s* jemi_encode(struct jemi_s* j, char const* s, size_t ns)
{
  jemi_quote(j);

  char const* ptr = s;
  char const* end = (ns == (size_t)-1) ? NULL : ptr + ns;
  char escape[6];

  while ((end && (ptr < end)) || (!end && ptr && *ptr)) {
    char const* pwrite = 0;
    switch (*ptr) {
    case '\"': pwrite = strcpy(escape, "\\\"");  break;
    case '\\': pwrite = strcpy(escape, "\\\\"); break;
    case '\b': pwrite = strcpy(escape, "\\b"); break;
    case '\f': pwrite = strcpy(escape, "\\f"); break;
    case '\n': pwrite = strcpy(escape, "\\n"); break;
    case '\r': pwrite = strcpy(escape, "\\r"); break;
    case '\t': pwrite = strcpy(escape, "\\t"); break;
    default:
      break;
    }

    if (!pwrite) {
      ptr ++;
      continue;
    }

    if (ptr > s) jemi_raw(j, s, ptr - s);
    jemi_raw(j, pwrite, strlen(pwrite));
    s = ++ptr;
  } //switch

  if (ptr > s) jemi_raw(j, s, ptr - s);
  return jemi_quote(j);
}

struct jemi_s* jemi_encode_c(struct jemi_s* j, char const* s)
{
  return jemi_encode(j, s, (size_t)-1);
}

struct jemi_s* jemi_integer(struct jemi_s* j, int64_t src)
{
 char s[80]; size_t ns = sizeof(s);
 snprintf(s, ns, "%" PRIi64, src);
 return jemi_raw_c(j, s);
}

struct jemi_s* jemi_uinteger(struct jemi_s* j, uint64_t src)
{
 char s[80]; size_t ns = sizeof(s);
 snprintf(s, ns, "%" PRIu64, src);
 return jemi_raw_c(j, s);
}

struct jemi_s* jemi_real(struct jemi_s* j, double src)
{
  double ip = 0.;
  double const fp = modf(fabs(src), &ip);

  if ((fp < DBL_EPSILON) || (FP_ZERO == fpclassify(fp)) ) {

    return jemi_integer(j, (int64_t)src);
  }

  char s[80]; int ns = sizeof(s);

  if (fp < 9e-09) {
    ns = snprintf(s, ns, "%g", src);
  } else {

    ns = snprintf(s, ns, "%.08f", src);
    if (ns > 0) {


      /* trail zero */
      char const* e = s + ns; {
        char const* dot = s;
        while (dot < e && *dot && *dot != '.') ++dot;
        while (e > dot && *(e - 1) == '0') --e;
        ns = e - s;
      }

      // dot
      if (ns && s[ns - 1] == '.')
        ns --;

    }
  }

  if (ns < 0) return jemi_null(j);
  return jemi_raw(j, s, ns);
}

struct jemi_s* jemi_boolean(struct jemi_s* j, int src)
{
  if (src) return jemi_raw_c(j, "true");
  return jemi_raw_c(j, "false");
}

struct jemi_s* jemi_object(struct jemi_s* j)
{
  assert(j->ndeep < sizeof(j->deep));
  j->deep[j->ndeep++] = '}';
  return jemi_raw_c(j, "{");
}

struct jemi_s* jemi_map_name_beg(struct jemi_s* j)
{
  assert(j->ndeep);
  if (j->count[j->ndeep - 1]) jemi_raw_c(j, ",");
  if (j->pretty) jemi_indent(jemi_raw_c(j, "\n"));
  return j;
}

struct jemi_s* jemi_map_name_end(struct jemi_s* j)
{
  j->count[j->ndeep - 1]++;
  return jemi_raw(j, ":", 1);
}

struct jemi_s* jemi_map(struct jemi_s* j, char const* n)
{  
  return jemi_map_name_end(jemi_plain_c(jemi_map_name_beg(j), n));
}

struct jemi_s* jemi_array(struct jemi_s* j)
{
  assert(j->ndeep < sizeof(j->deep));
  j->deep[j->ndeep++] = ']';
  return jemi_raw_c(j, "[");
}

struct jemi_s* jemi_value(struct jemi_s* j)
{
  assert(j->ndeep);
  if (j->count[j->ndeep - 1]++) jemi_raw_c(j, ",");
  if (j->pretty) jemi_indent(jemi_raw_c(j, "\n"));
  return j;
}

struct jemi_s* jemi_pop(struct jemi_s* j)
{
  assert(j->ndeep);
  j->ndeep --;
  if (j->pretty && j->count[j->ndeep]) jemi_indent(jemi_raw_c(j, "\n"));
  jemi_raw(j, &j->deep[j->ndeep ], 1);
  j->count[j->ndeep] = 0;
  j->deep[j->ndeep] = 0;
  return j;
}

struct jemi_s* jemi_done(struct jemi_s* j)
{
  while (j->ndeep) jemi_pop(j);
  return jemi_flush(j);
}

