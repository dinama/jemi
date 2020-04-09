#pragma once

#include <cstddef>
#include <string>
#include <ostream>
#include "jemi.h"

namespace jemi {

struct object{};
struct array{};
struct pop{};
struct done{};
struct pretty{};

template<typename T>
struct quote {
  T v;
  quote(T src) : v(src) {}
};

struct json {
  char const* s;
  size_t ns;
  json(char const* src, size_t nsrc) : s(src), ns(nsrc) {}
};

template<typename T>
struct encode {
  T v;
  encode(T src) : v(src) {}
};

template<typename T>
inline quote<T> quoted(T src)
{ return quote<T>(src); }

template<typename T>
inline encode<T> encoded(T src)
{ return encode<T>(src); }

inline json jsoned(char const* s, size_t ns) {
  return json(s, ns);
}

inline json jsoned(char const* s) {
  return json(s, (size_t)-1);
}

inline json jsoned(std::string const&  s) {
  return json(s.data(), s.size());
}

template<typename T>
struct emitterT : jemi_s {

  // T must implement:
  // write(char const*, size_t);

  emitterT();
  // ~emitterT();

  operator T&();
  template<typename V>
  T& operator >> (V);
  T& operator << (jemi::pretty);
  T& operator << (jemi::pop);
  T& operator << (jemi::done);
  template<typename V>
  T& operator << (V);
  T& __emit(jemi::object);
  T& __emit(jemi::array);
  template<typename V>
  T& __emit(quote<V> const&);
  T& __emit(json const&);
  T& __emit(encode<char const*> const&);
  T& __emit(encode<std::string> const&);
  T& __emit(std::nullptr_t);
  T& __emit(bool b);
  template<size_t N>
  T& __emit(char (&)[N]);
  template<typename V>
  T& __emit(V);
  template<typename V>
  void __sub_emit(V src, void*, int*);
  template<typename V>
  void __sub_emit(V src, double*, void*);
  void __sub_emit(char const*, void*, void*);
  void __sub_emit(std::string const&, void*, void*);
};

template<typename T>
emitterT<T>::emitterT()
{
  jemi_init_ex(this, [](char const* s, size_t ns, void* p) {
    ((T*)p)->write(s, ns); }, this);
}

// template<typename T>
// emitterT<T>::~emitterT()
// { jemi_done(this); }

template<typename T>
emitterT<T>::operator T&()
{ return *((T*)this); }

template<typename T>
template<typename V>
T& emitterT<T>::operator >> (V val)
{
  jemi_map_name_beg(this);
  *this << val;
  jemi_map_name_end(this);
  return *this;
}

template<typename T>
T& emitterT<T>::operator << (jemi::pretty)
{
  pretty = 1;
  return *this;
}

template<typename T>
T& emitterT<T>::operator << (jemi::pop)
{
  jemi_pop(this);
  return *this;
}

template<typename T>
T& emitterT<T>::operator << (jemi::done)
{
  jemi_done(this);
  return *this;
}

template<typename T>
template<typename V>
T& emitterT<T>::operator << (V val)
{
  if (ndeep && ']' == deep[ndeep - 1]) jemi_value(this);
  return __emit(val);
}

template<typename T>
T& emitterT<T>::__emit(jemi::object)
{
  jemi_object(this);
  return *this;
}

template<typename T>
T& emitterT<T>::__emit(jemi::array)
{
  jemi_array(this);
  return *this;
}

template<typename T>
template<typename V>
T& emitterT<T>::__emit(quote<V> const& src)
{
  jemi_raw_c(this, "\"");
  *this << (src.v);
  jemi_raw_c(this, "\"");
  return *this;
}

template<typename T>
T& emitterT<T>::__emit(json const& src)
{
  if (((size_t)-1) == src.ns) jemi_json_c(this, src.s);
  else jemi_json(this, src.s, src.ns);
  return *this;
}

template<typename T>
T& emitterT<T>::__emit(encode<char const*> const& src)
{
  jemi_encode_c(this, src.v);
  return *this;
}

template<typename T>
T& emitterT<T>::__emit(encode<std::string> const& src)
{
  jemi_encode(this, src.v.data(), src.v.size() );
  return *this;
}

template<typename T>
T& emitterT<T>::__emit(std::nullptr_t)
{
  jemi_null(this);
  return *this;
}

template<typename T>
T& emitterT<T>::__emit(bool b)
{
  jemi_boolean(this, b ? 1 : 0);
  return *this;
}

template<typename T>
template<size_t N>
T& emitterT<T>::__emit(char (&src)[N])
{
  jemi_plain(this, src, N);
  return *this;
}


template<typename T>
template<typename V>
T& emitterT<T>::__emit(V src)
{
  constexpr typename std::conditional<std::is_floating_point<V>::value, double*, void*>::type v1 = nullptr;
  constexpr typename std::conditional<std::is_integral<V>::value, int*, void*>::type v2  = nullptr;
  __sub_emit(src, v1, v2);
  return *this;
}

template<typename T>
template<typename V>
void emitterT<T>::__sub_emit(V src, void*, int*)
{
  if (std::is_signed<V>::value) jemi_integer(this, (int64_t)src);
  else jemi_uinteger(this, (uint64_t)src);
}

template<typename T>
template<typename V>
void emitterT<T>::__sub_emit(V src, double*, void*)
{ jemi_real(this, (double)src); }

template<typename T>
void emitterT<T>::__sub_emit(char const* src, void*, void*)
{ jemi_plain_c(this, src); }

template<typename T>
void emitterT<T>::__sub_emit(std::string const& src, void*, void*)
{
  jemi_plain(this, src.data(), src.size());
}

}
