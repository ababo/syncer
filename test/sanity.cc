#include <regex>
#include <thread>

#include "catch.hpp"
#include "data.h"
#include "syncer.h"

namespace syncer {
namespace test {

TEST_CASE("sanity") {
  Data data;
  Server<Data> server("tcp://*:5000", "tcp://*:5001", data);

  data.ints = { 1, 2, 3 };
  data.items["key"] = { 123, "hello" };
  data.baz = 321;
  server.Update(data);

  auto fired = 0;
  PatchOpRouter<Data> router;
  router.AddCallback<int>(R"(/ints/(\d+))", PATCH_OP_ADD,
    [&] (const Data&, const smatch& match, PatchOp op, int val) {
      REQUIRE(op == PATCH_OP_ADD);
      REQUIRE(val == data.ints[stoi(match[1].str())]);
      fired++;
    });
  router.AddCallback<Item>(R"(/items/(\w+))", PATCH_OP_ANY,
    [&] (const Data&, const smatch& match, PatchOp op, const Item& val) {
      REQUIRE(op == PATCH_OP_ADD);
      REQUIRE(data.items[match[1].str()] == data.items["key"]);
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

  REQUIRE(fired == 5);
  REQUIRE(client.With<Data>([](const Data& data) { return data; }) == data);
}

}
}
