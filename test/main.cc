#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "syncer.h"

namespace syncer {
namespace test {

struct Data {
  int version;
};

void from_json(const json& j, Data& d) {

}

void to_json(json& j, const Data& d) {

}

TEST_CASE("client/server sanity") {
  PatchOpRouter<Data> router;
  Client<Data> client("tcp://*:5000", "tcp://*:5001", router);
}

}
}
