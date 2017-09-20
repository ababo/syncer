/**
 * @file client.h
 * @author Simon Prykhodko
 * @brief Synchronized client implementation.
 */

#ifndef SYNCER_CLIENT_H_
#define SYNCER_CLIENT_H_

#include <chrono>
#include <mutex>
#include <thread>

#include "backend.h"
#include "json.hpp"
#include "patch_op_router.h"

namespace syncer {

using namespace std;
using namespace chrono;
using namespace std::placeholders;

/**
 * @brief Synchronized client.
 * @details It creates both requester and subscriber at time of construction.
 * Then the requester retrieves a full data state from a remote server.
 *
 * When the data is changed the subscriber will receive notifications
 * containing the corresponding JSON patches (RFC 6902) and will apply them in
 * order to make the local data match the remote counterpart.
 *
 * Before applying a patch the client checks its version to ensure data
 * consistency. If the version is wrong the client requests a full data state.
 * Otherwise it handles the patch operations by a router which was supplied
 * during construction.
 *
 * Requirements for data type template parameter:
 *   - Must have a default constructor.
 *   - Must have `from_json` and `to_json` function overloads.
 */
template <typename T, typename Backend = DefaultBackend> class Client {
 public:
  /** @brief Time period in milliseconds to retry after failure. */
  static const int RETRY_PERIOD = 5000;

  /**
   * @brief Constructor.
   * @param req_conf a requester configuration.
   * @param sub_conf a subscriber configuration.
   * @param router a patch operation router (with callbacks added).
   */
  Client(const typename Backend::Config& req_conf,
         const typename Backend::Config& sub_conf,
         const PatchOpRouter<T>& router)
      : req_(req_conf, bind(&Client::HandleReply, ref(*this), _1, _2))
      , sub_(sub_conf, bind(&Client::HandleNotification, ref(*this), _1))
      , router_(router) {
    req_.Request(Message());
  }

  /**
   * @brief Thread-safe data accessor.
   * @param func a function to access data.
   */
  void Do(function<void(const T& data)> func) const {
    lock_guard<mutex> _(const_cast<mutex&>(mtx_));
    func(data_);
  }

  /**
   * @brief Thread-safe data accessor with value returned.
   * @param func a function to access data.
   */
  template <typename T2> T2 With(function<T(const T& data)> func) const {
    lock_guard<mutex> _(const_cast<mutex&>(mtx_));
    return func(data_);
  }

 private:
  void HandleReply(bool success, const Message& msg) {
    if (!success) {
      int period = RETRY_PERIOD;
      thread([this, period]() {
        this_thread::sleep_for(milliseconds(period));
        req_.Request(Message());
      }).detach();
      return;
    }

    T data;
    auto after = json::parse(msg);
    ver_ = after["__syncer_data_version"];
    from_json(after, data);

    lock_guard<mutex> _(mtx_);
    json before;
    to_json(before, data_);
    HandleDiff(json::diff(before, after));
    data_ = data;
  }

  void HandleNotification(const Message& msg) {
    auto diff = json::parse(msg);

    lock_guard<mutex> _(mtx_);
    for (auto it = diff.begin(); it != diff.end(); ++it) {
      if ((*it)["path"] == "/__syncer_data_version") {

        if ((*it)["value"] == ver_ + 1) {
          HandleDiff(diff);

          json before;
          to_json(before, data_);
          json after = before.patch(diff);
          from_json(after, data_);
          ver_++;

        } else {
          req_.Request(Message());
        }

        break;
      }
    }
  }

  void HandleDiff(const json& diff) {
    for (auto it = diff.begin(); it != diff.end(); ++it) {
      PatchOp op;
      string ops = (*it)["op"];
      if (ops == "add") {
        op = PATCH_OP_ADD;
      } else if (ops == "remove") {
        op = PATCH_OP_REMOVE;
      } else if (ops == "replace") {
        op = PATCH_OP_REPLACE;
      } else {
        continue;
      }

      json val((*it)["value"]);
      router_.HandleOp(data_, (*it)["path"], op, val);
    }
  }

  typename Backend::Requester req_;
  typename Backend::Subscriber sub_;
  PatchOpRouter<T> router_;
  T data_;
  int ver_ = 0;
  mutex mtx_;
};

}

#endif // SYNCER_CLIENT_H_
