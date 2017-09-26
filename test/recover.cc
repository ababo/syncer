#include <chrono>
#include <list>
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
 * 2. Create a list of 10 clients.
 * 3. Make sure the clients have updated their data state.
 * 4. Destroy the server and create a new one.
 * 5. Modify the data state.
 * 6. Make sure the clients have updated their data state.
 */
TEST_CASE("recover") {
  thread thread([&] {
    Data data;
    data.baz = 1;
    Server<Data> server("tcp://*:5000", "tcp://*:5001", data);
    this_thread::sleep_for(milliseconds(100));
  });

  PatchOpRouter<Data> router;
  list<Client<Data>> clients;
  for (int i = 0; i < 10; i++) {
    clients.emplace_back("tcp://localhost:5000",
                         "tcp://localhost:5001", router);
  }
  this_thread::sleep_for(milliseconds(100));

  thread.join();
  for (auto& client : clients) {
    REQUIRE(client.data().baz == 1);
  }

  Data data;
  data.baz = 2;
  Server<Data> server("tcp://*:5000", "tcp://*:5001", data);
  int period = DefaultSocket::PUB_SUB_CONNECT_PERIOD + 100;
  this_thread::sleep_for(milliseconds(period));

  for (const auto& client : clients) {
    REQUIRE(client.data().baz == 2);
  }
}

}
}
