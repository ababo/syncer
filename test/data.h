#ifndef SYNCER_TEST_DATA_H_
#define SYNCER_TEST_DATA_H_

#include <map>
#include <vector>

#include "json.hpp"

namespace syncer {
namespace test {

using namespace nlohmann;
using namespace std;

struct Item {
  int foo;
  string bar;
};

static inline bool operator==(const Item& a, const Item& b) {
  return a.foo == b.foo && a.bar == b.bar;
}

static inline void to_json(json& j, const Item& c) {
  j = json();
  j["foo"] = c.foo;
  j["bar"] = c.bar;
}

static inline void from_json(const json& j, Item& c) {
  c.foo = j.at("foo").get<int>();
  c.bar = j.at("bar").get<string>();
}

struct Data {
  vector<int> ints;
  map<string, Item> items;
  int baz;
};

static inline void to_json(json& j, const Data& c) {
  j = json();
  j["ints"] = c.ints;
  j["items"] = c.items;
  j["baz"] = c.baz;
}

static inline void from_json(const json& j, Data& c) {
  c.ints = j.at("ints").get<vector<int>>();
  c.items = j.at("items").get<map<string, Item>>();
  c.baz = j.at("baz").get<int>();
}

static inline bool operator==(const Data& a, const Data& b) {
  return a.ints == b.ints && a.items == b.items && a.baz == b.baz;
}

}
}

#endif // SYNCER_TEST_DATA_H_
