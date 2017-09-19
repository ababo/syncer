#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "syncer.h"

using namespace syncer;

struct Data {

};

TEST_CASE("client/server sanity") {
  PatchOpHandler<Data> handler;
  Client<Data> client("tcp://*:5000", "tcp://*:5001", handler);
}
