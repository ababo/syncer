#include <atomic>
#include <chrono>
#include <thread>

#include "catch.hpp"
#include "data.h"
#include "syncer.h"

namespace syncer {
namespace test {

using namespace std;
using namespace std::chrono;

/*
 * 1. Create a publisher with a specified subject.
 * 2. Create two subscribers: with a same and a different subjects.
 * 3. Publish some message.
 * 4. Make sure the first subscriber receives the message.
 * 5. Make sure the second subscriber doesn't receive anything.
 */
TEST_CASE("pubsub") {
  using Params = DefaultSocket::Params;
  using Message = DefaultSocket::Message;

  Publisher<> pub("tcp://*:5000");

  atomic_bool rec1(false);
  auto params = Params("tcp://localhost:5000", "subj1");
  Subscriber<> sub1(params, [&] (const Message& msg) {
    cout << msg.body() << endl;
    rec1 = string(msg.subject()) == "subj1" && string(msg.body()) == "hello";
  });

  atomic_bool rec2(false);
  params = Params("tcp://localhost:5000", "subj2");
  Subscriber<> sub2(params, [&] (const Message& msg)  {
    rec2 = true;
  });

  int period = DefaultSocket::PUB_SUB_CONNECT_PERIOD;
  this_thread::sleep_for(chrono::milliseconds(period));

  pub.Publish(DefaultSocket::Message("subj1", "hello"));

  this_thread::sleep_for(chrono::milliseconds(100));

  REQUIRE(rec1);
  REQUIRE(!rec2);
}

}
}
