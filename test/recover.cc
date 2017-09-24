#include <chrono>
#include <thread>

#include "catch.hpp"
#include "data.h"
#include "syncer.h"

namespace syncer {
namespace test {

using namespace std;
using namespace std::chrono;

TEST_CASE("recover") {
  thread thread([&] {
    Data data;
    data.baz = 1;
    Server<Data> server("tcp://*:5000", "tcp://*:5001", data);
    this_thread::sleep_for(milliseconds(100));
  });

  PatchOpRouter<Data> router;
  Client<Data> client("tcp://localhost:5000", "tcp://localhost:5001", router);
  this_thread::sleep_for(milliseconds(100));

  thread.join();
  REQUIRE(client.data().baz == 1);

  Data data;
  data.baz = 2;
  Server<Data> server("tcp://*:5000", "tcp://*:5001", data);
  this_thread::sleep_for(milliseconds(100));

  REQUIRE(client.data().baz == 2);
}

}
}
