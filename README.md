# Syncer - the library for state synchronization

### What's that?

Syncer is a small C++11 library which helps to manage communication between micro-services. It simplifies sharing server's state between multiple clients as well as handling events when this state is being changed.

Let's say, you have a service (process) which periodically reads data from multiple sensors. This info should be available for many other services in real time, moreover they need to trigger some actions depending on different data changes.

Of course we can employ "public/subscribe" communication pattern using a message bus: the sensor-equipped server will publish its state each time when it changes, and the clients will subscribe and receive the corresponding messages. For small and simple state it will work perfectly. But what if the state becomes huge and complex? It doesn't make much sense to publish it each time of alteration. Moreover in such a case it's very hard to track specific changes.

This is a perfect use case for Syncer:

```cpp
#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "syncer.h"

using namespace nlohmann;
using namespace std;
using namespace std::chrono;
using namespace syncer;

struct Site {
  int temperature;
  int pressure;
};

static inline void to_json(json& j, const Site& s) {
  j = json();
  j["temperature"] = s.temperature;
  j["pressure"] = s.pressure;
}

static inline void from_json(const json& j, Site& s) {
  s.temperature = j.at("temperature").get<int>();
  s.pressure = j.at("pressure").get<int>();
}

struct State {
  map<string, Site> sites;
  string forecast;
};

static inline void to_json(json& j, const State& s) {
  j = json();
  j["sites"] = s.sites;
  j["forecast"] = s.forecast;
}

static inline void from_json(const json& j, State& s) {
  s.sites = j.at("sites").get<map<string, Site>>();
  s.forecast = j.at("forecast").get<string>();
}

void AddEventHandlers(PatchOpRouter<State>& router) {
  router.AddCallback<int>(R"(/sites/(\w+)/temperature)", PATCH_OP_ANY,
    [] (const State& old, const smatch& m, PatchOp op, int t) {
      cout << "Temperature in " << m[1].str() << " has changed: "
           << old.sites.at(m[1].str()).temperature << " -> " << t << endl;
    });

  router.AddCallback<Site>(R"(/sites/(\w+)$)", PATCH_OP_ADD,
    [&] (const State&, const smatch& m, PatchOp op, const Site& s) {
      cout << "Site added: " << m[1].str()
           << " (temperature: " << s.temperature
           << ", pressure: " << s.pressure << ")" << endl;
    });

  router.AddCallback<Site>(R"(/sites/(\w+)$)", PATCH_OP_REMOVE,
    [&] (const State&, const smatch& m, PatchOp op, const Site&) {
      cout << "Site removed: " << m[1].str() << endl;
    });
}

int main() {
  State state;
  state.sites["forest"] = { 51, 29 };
  state.sites["lake"] = { 49, 31 };
  state.forecast = "cloudy and rainy";
  Server<State> server("tcp://*:5000", "tcp://*:5001", state);

  PatchOpRouter<State> router;
  AddEventHandlers(router);
  Client<State> client("tcp://localhost:5000", "tcp://localhost:5001", router);

  this_thread::sleep_for(milliseconds(100));

  cout << "Forecast: " << client.data().forecast << endl;

  state.sites.erase("lake");
  state.sites["forest"] = { 50, 28 };
  state.sites["desert"] = { 55, 30 };
  state.forecast = "cloudy and rainy";
  server.Update(state);

  this_thread::sleep_for(milliseconds(100));

  return 0;
}
```

You'll have the following output:
```
Site added: forest (temperature: 51, pressure: 29)
Site added: lake (temperature: 49, pressure: 31)
Forecast: cloudy and rainy
Temperature in forest has changed: 51 -> 50
Site removed: lake
Site added: desert (temperature: 55, pressure: 30)
```

As you see it's easy to add callbacks to handle arbitrary events related to data state changes. All you need to do is to specify [JSON Patch](http://jsonpatch.com/) operations to fire on, a regular expression to match the corresponding paths and a callback itself.

### What else can you do with Syncer?

Beside the `Server` and `Client` Syncer provides a set of helper classes which can be handy for other use cases:

- `Publisher` - allows to publish an arbitrary data.
- `Subscriber` - allows to process incoming notifications via provided callback\*.
- `Requester` - allows to send requests, handling the corresponding replies via provided callback\*.
- `Replier` - allows to process incoming requests via provided callback, sending the corresponding replies\*.

  <b>\*</b> The callback will be called sequentially in a dedicated thread

The typical use case for these classes is when you need to transfer binary data efficiently (the `Server` and `Client` use JSON for data transport, which is not suitable here).

### How to install?

Syncer is a header-only library, so there's no need to build it separately. But Syncer depends on a low-level messaging backend. Currently only ZeroMQ 4.x is supported as a backend. So it needs to be installed through your package manager or from a source code.

Actually Syncer depends on the following libraries:

1. [ZeroMQ 4.x](http://zeromq.org/) (needs to be installed separately).
2. [JSON](https://github.com/nlohmann/json) (already included).
3. [Catch](https://github.com/philsquared/Catch) (already included).

As was said, Syncer doesn't need to be built separately. But you can build a Doxygen documentation by `make open-doc` or run unit-tests by `make run-test`.
