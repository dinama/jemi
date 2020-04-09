#include <mutest.h>
#include <string>
#include <sstream>
#include <cstdio>
#include <jemi.h>
#include <jemi++.h>


namespace test {
  static void string_write(char const* s, size_t ns, void* p) {
    ((std::string*)p)->append(s, ns);
  }
}

extern "C" void mu_test_256() {
  std::string r;
  std::string m;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);
  jemi_array(&j);
  m = "[";
  for (int i = 0; i < 1024; ++i) {
    auto s = !(i % 256) ? "X" : "-";
    jemi_encode_c(jemi_value(&j), s);
    if (0 == i) m.append("\"").append(s).append("\"");
    else m.append(",").append("\"").append(s).append("\"");
  }
  jemi_done(&j);
  m.append("]");

  mu_check(!m.compare(r));
}

extern "C" void mu_test_encodesized()
{
  char b[] = {"plain and plain"};
  std::string r;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);
  jemi_encode_c(&j, b);
  jemi_done(&j);

  mu_check(0 == r.compare("\"plain and plain\""));

  r.clear();
  jemi_init_ex(&j, test::string_write, &r);
  jemi_encode(&j, b, sizeof("plain") - 1);
  jemi_done(&j);

  mu_check(0 == r.compare("\"plain\""));

}

extern "C" void mu_test_write()
{
  std::string r;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);

  char b[1024];
  for (size_t i = 0; i < sizeof(b); ++i) b[i] = 'A';

  jemi_write(&j, b, sizeof(b));
  jemi_write(&j, b, sizeof(b));

  mu_check(j.nwrited == sizeof(b) * 2);
  mu_check(j.nwrited == r.size());
}

extern "C" void mu_test_raw()
{
  std::string r;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);

  char b[1024];
  for (size_t i = 0; i < sizeof(b); ++i) b[i] = 'A';

  jemi_raw(&j, b, sizeof(b));
  jemi_raw(&j, b, sizeof(b));
  jemi_done(&j);

  mu_check(j.nwrited == sizeof(b) * 2);
  mu_check(j.nwrited == r.size());
}

extern "C" void mu_test_primitives()
{
  std::string r;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);

  jemi_nindent(&j, 1);
  jemi_json(&j, "{\n}", sizeof("{\n}") - 1);
  jemi_json_c(&j, "{\n}");
  jemi_null(&j);
  jemi_quote(&j);
  jemi_plain(&j, "plain", sizeof("plain") - 1);
  jemi_plain_c(&j, "plain");
  jemi_encode(&j, "\b", 1);
  jemi_encode_c(&j, "\b");
  jemi_integer(&j, 10);
  jemi_real(&j, 9.9);
  jemi_boolean(&j, false);
  jemi_done(&j);

  mu_check(0 == r.compare("  {\n}{\n}null\"\"plain\"\"plain\"\"\\b\"\"\\b\"109.9false"));
}

extern "C" void mu_test_objects()
{
  std::string r;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);

  jemi_array(&j);
  jemi_integer(jemi_value(&j), 1);
  jemi_plain_c(jemi_value(&j), "plain");

  jemi_object(jemi_value(&j));
  jemi_integer(jemi_map(&j, "id"), 1);
  jemi_plain_c(jemi_map(&j, "name"), "bob");
  jemi_pop(&j);

  jemi_object(jemi_value(&j));
  jemi_done(&j);

  mu_check(0 == r.compare("[1,\"plain\",{\"id\":1,\"name\":\"bob\"},{}]"));
}

extern "C" void mu_test_pretty()
{
  std::string r;

  struct jemi_s j;
  jemi_init_ex(&j, test::string_write, &r);
  j.pretty = 1;

  jemi_array(&j);
  jemi_integer(jemi_value(&j), 1);
  jemi_integer(jemi_value(&j), 2);
  jemi_plain_c(jemi_value(&j), "plain");

  jemi_object(jemi_value(&j));
  jemi_integer(jemi_map(&j, "id"), 1);
  jemi_plain_c(jemi_map(&j, "name"), "bob");
  jemi_array(jemi_map(&j, "arr"));
  jemi_integer(jemi_value(&j), 1);
  jemi_pop(&j);
  jemi_pop(&j);

  jemi_object(jemi_value(&j));
  jemi_pop(&j);
  jemi_array(jemi_value(&j));
  jemi_done(&j);

  char const* s =
"[" "\n"
"  1," "\n"
"  2," "\n"
"  \"plain\"," "\n"
"  {" "\n"
"    \"id\":1," "\n"
"    \"name\":\"bob\"," "\n"
"    \"arr\":[" "\n"
"      1" "\n"
"    ]" "\n"
"  }," "\n"
"  {}," "\n"
"  []" "\n"
"]";

  mu_check(0 == r.compare(s));
}

extern "C" void mu_test_cpp()
{

  struct stringer : jemi::emitterT<stringer> {

    std::string& r;

    stringer(std::string& src)
      : r(src) { r.clear(); }

    void write(char const* s, size_t ns)
    { r.append(s, ns); }
  };

  std::string r;

  /* object & end */

  stringer(r.assign(""))
    << jemi::object() >> "name" << "bob" << jemi::pop() << jemi::done();
  mu_check(0 == r.compare("{\"name\":\"bob\"}"));

  /* object & done */

  stringer(r.assign(""))
    << jemi::object() >> "name" << "bob" << jemi::done();
  mu_check(0 == r.compare("{\"name\":\"bob\"}"));

  /* array  */

  stringer(r.assign(""))
    << jemi::array() << 1 << 2 << 3 << jemi::done();
  mu_check(0 == r.compare("[1,2,3]"));

  /* complex array  */

  stringer(r.assign(""))
      << jemi::array() <<
         jemi::object() >> "id" << 1 << jemi::pop()
      << 3 <<
         jemi::object() >> "id" << 2 << jemi::pop()
      << 4 << 5 << "hi"  << jemi::done();

  mu_check(0 == r.compare("[{\"id\":1},3,{\"id\":2},4,5,\"hi\"]"));

  /* complex object */

  stringer(r.assign(""))
      << jemi::object()
        >> "child1"  << jemi::object() >> "id" << 1 << jemi::pop()
        >> "child2"  << jemi::object() >> "id" << 2  << jemi::done();


  mu_check(0 == r.compare("{\"child1\":{\"id\":1},\"child2\":{\"id\":2}}"));

  /* pretty   */

  stringer(r.assign("")) << jemi::pretty()
    << jemi::object() >> "name" << "bob"  << jemi::done();


  mu_check(0 == r.compare("{\n  \"name\":\"bob\"\n}"));


  /*  stroke */

  stringer(r.assign(""))
    << jemi::object() >> "name" << jemi::quoted(1)  << jemi::done();
  mu_check(0 == r.compare("{\"name\":\"1\"}"));

  stringer(r.assign(""))
      <<  jemi::object() >> "v" << false  << jemi::done();

  mu_check(0 == r.compare("{\"v\":false}"));

  /*  nullptr */

  stringer(r.assign(""))
      <<  jemi::object() >> "p" << nullptr  << jemi::done();

  mu_check(0 == r.compare("{\"p\":null}"));


  /*  encode */

  stringer(r.assign(""))
      <<  jemi::array() <<  jemi::encoded("\b")  << jemi::done();

  /*  encode */

  std::string s("\b\t");
  stringer(r.assign(""))
      <<  jemi::array() <<  jemi::encoded(s) << jemi::done();

  mu_check(0 == r.compare("[\"\\b\\t\"]"));

  /*  json */

  stringer(r.assign(""))
      <<  jemi::array() <<  jemi::jsoned("[1, 2, 3]") <<  jemi::jsoned("[1, 2]")
       << jemi::done();

  mu_check(0 == r.compare("[[1, 2, 3],[1, 2]]"));

  /*  not temporary object */
  {
    stringer ss(r.assign(""));
    auto& o = ss << jemi::object();
    auto& p = o >> "name";
    p << "bob";
    mu_check(r.empty());
    ss << jemi::done();

    //puts(r.c_str());
    mu_check(0 == r.compare("{\"name\":\"bob\"}"));
  }

}
