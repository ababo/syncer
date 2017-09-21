/**
 * @file client.h
 * @author Simon Prykhodko
 * @brief Synchronized client implementation.
 */

#ifndef SYNCER_CLIENT_H_
#define SYNCER_CLIENT_H_

#include <mutex>

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
 *   - Might have a move constructor (can boost performance).
 */
template <typename T, typename Backend = DefaultBackend> class Client {
 public:
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
    T data;
    to_json(state_, data);
    state_[VERSION_KEY] = 0;

    req_.Request(Message());
  }

  /**
   * @brief Data accessor.
   * @return a current data state.
   */
  T data() const {
    T data;
    {
      lock_guard<mutex> _(const_cast<mutex&>(mtx_));
      from_json(state_, data);
    }
    return move(data);
  }

 private:
  static constexpr char const * VERSION_PATH = "/__syncer_data_version";
  static constexpr char const * VERSION_KEY = &VERSION_PATH[1];

  void HandleReply(bool success, const Message& msg) {
    if (!success) {
      SYNCER_LOG_FMT("failed to receive server's reply");
      return;
    }

    json after = json::parse(msg);

    {
      lock_guard<mutex> _(mtx_);
      HandleDiff(json::diff(state_, after));
      state_ = after;
    }
  }

  void HandleNotification(const Message& msg) {
    if (msg.empty()) {
      req_.Request(Message());
      return;
    }

    auto diff = json::parse(msg);

    for (auto it = diff.begin(); it != diff.end(); ++it) {
      if ((*it)["path"] == VERSION_PATH) {
        lock_guard<mutex> _(mtx_);

        int ver = state_[VERSION_KEY];
        if ((*it)["value"] == ver + 1) {
          HandleDiff(diff);
          state_ = state_.patch(diff);
        } else {
          req_.Request(Message());
        }

        break;
      }
    }
  }

  void HandleDiff(const json& diff) {
    T data;
    from_json(state_, data);

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

      json val;
      if (op != PATCH_OP_REMOVE) {
        val = ((*it)["value"]);
      }
      router_.HandleOp(data, (*it)["path"], op, val);
    }
  }

  typename Backend::Requester req_;
  typename Backend::Subscriber sub_;
  PatchOpRouter<T> router_;
  json state_;
  mutex mtx_;
};

}

#endif // SYNCER_CLIENT_H_
