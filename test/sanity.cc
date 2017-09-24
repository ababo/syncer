#include <atomic>
#include <chrono>
#include <regex>
#include <thread>

#define JSON_NOEXCEPTION

#include "catch.hpp"
#include "data.h"
#include "syncer.h"

namespace syncer {
namespace test {

using namespace std;
using namespace std::chrono;

TEST_CASE("sanity") {
  Data data;
  data.ints = { 1, 2, 3 };
  data.items["key"] = Item(123, "hello");
  data.baz = 321;
  Server<Data> server("tcp://*:5000", "tcp://*:5001", data);

  atomic_int fired(0);
  PatchOpRouter<Data> router;
  router.AddCallback<int>(R"(/ints/(\d+))", PATCH_OP_ADD,
    [&] (const Data&, const smatch& match, PatchOp op, int val) {
      REQUIRE(op == PATCH_OP_ADD);
      REQUIRE(val == data.ints[stoi(match[1].str())]);
      fired++;
    });
  router.AddCallback<Item>(R"(/ints/(\w+))", PATCH_OP_REMOVE,
    [&] (const Data&, const smatch& match, PatchOp op, const Item&) {
      REQUIRE(op == PATCH_OP_REMOVE);
      REQUIRE(stoi(match[1].str()) == 2);
      fired++;
    });
  router.AddCallback<Item>(R"(/items/(\w+))", PATCH_OP_ADD,
    [&] (const Data&, const smatch& match, PatchOp op, const Item& val) {
      REQUIRE(op == PATCH_OP_ADD);
      auto key = match[1].str();
      REQUIRE(data.items[key] == data.items[key]);
      fired++;
    });
  router.AddCallback<int>(R"(/baz)", PATCH_OP_REPLACE,
    [&] (const Data&, const smatch& match, PatchOp op, int val) {
      REQUIRE(op == PATCH_OP_REPLACE);
      REQUIRE(data.baz == val);
      fired++;
    });
  Client<Data> client("tcp://localhost:5000", "tcp://localhost:5001", router);
  this_thread::sleep_for(milliseconds(100));

  data.ints.pop_back();
  data.items["key2"] = Item(234, "bye");
  data.baz = 432;
  server.Update(data);
  this_thread::sleep_for(milliseconds(100));

  REQUIRE(fired == 8);
  REQUIRE(client.data() == data);
}

}
}
