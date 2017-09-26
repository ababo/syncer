#include <atomic>
#include <chrono>
#include <regex>
#include <thread>

#include "catch.hpp"
#include "data.h"
#include "syncer.h"

namespace syncer {
namespace test {

using namespace std;
using namespace std::chrono;

/*
 * 1. Create a server.
 * 2. Create 10 clients, each one in a dedicated thread.
 * 3. Update the server's data state.
 * 4. Make sure all the clients have received that update.
 */
TEST_CASE("stress") {
  Data data;
  Server<Data> server("tcp://*:5000", "tcp://*:5001", data);

  atomic_int fired(0);
  PatchOpRouter<Data> router;
  router.AddCallback<int>(R"(/baz)", PATCH_OP_REPLACE,
    [&] (const Data&, const smatch& match, PatchOp op, int val) {
      fired++;
    });

  std::vector<std::thread> threads;
  for (int i = 0; i < 10; i++) {
    threads.push_back(std::thread([&] {
      Client<Data> client("tcp://localhost:5000",
                          "tcp://localhost:5001", router);
      this_thread::sleep_for(milliseconds(200));
    }));
  }

  this_thread::sleep_for(milliseconds(100));

  for (int i = 0; i < 100; i++) {
    data.baz++;
    server.Update(data);
  }

  for (auto& thr : threads) {
    thr.join();
  }

  REQUIRE(fired == 1000);
}

}
}
