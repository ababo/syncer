#ifndef SYNCER_TEST_DATA_H_
#define SYNCER_TEST_DATA_H_

#include <map>
#include <string>
#include <vector>

#include "json.hpp"

namespace syncer {
namespace test {

struct Item {
  int foo;
  std::string bar;
};

static inline bool operator==(const Item& a, const Item& b) {
  return a.foo == b.foo && a.bar == b.bar;
}

static inline void to_json(nlohmann::json& j, const Item& c) {
  j = nlohmann::json();
  j["foo"] = c.foo;
  j["bar"] = c.bar;
}

static inline void from_json(const nlohmann::json& j, Item& c) {
  c.foo = j.at("foo").get<int>();
  c.bar = j.at("bar").get<std::string>();
}

struct Data {
  std::vector<int> ints;
  std::map<std::string, Item> items;
  int baz;
};

static inline void to_json(nlohmann::json& j, const Data& c) {
  j = nlohmann::json();
  j["ints"] = c.ints;
  j["items"] = c.items;
  j["baz"] = c.baz;
}

static inline void from_json(const nlohmann::json& j, Data& c) {
  c.ints = j.at("ints").get<std::vector<int>>();
  c.items = j.at("items").get<std::map<std::string, Item>>();
  c.baz = j.at("baz").get<int>();
}

static inline bool operator==(const Data& a, const Data& b) {
  return a.ints == b.ints && a.items == b.items && a.baz == b.baz;
}

}
}

#endif // SYNCER_TEST_DATA_H_
