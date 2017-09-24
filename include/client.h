/**
 * @file client.h
 * @author Simon Prykhodko
 * @brief Synchronized client implementation.
 */

#ifndef SYNCER_CLIENT_H_
#define SYNCER_CLIENT_H_

#include <mutex>

#include "socket.h"
#include "json.hpp"
#include "patch_op_router.h"
#include "requester.h"
#include "subscriber.h"

namespace syncer {

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
template <typename T, typename Socket = DefaultSocket> class Client {
 public:
  /** @brief Alias for socket parameters. */
  using Params = typename Socket::Params;

  /** @brief Alias for socket message. */
  using Message = typename Socket::Message;

  /**
   * @brief Constructor.
   * @param req_params requester parameters.
   * @param sub_params subscriber parameters.
   * @param router a patch operation router (with callbacks added).
   */
  Client(const Params& req_params,
         const Params& sub_params,
         const PatchOpRouter<T>& router)
      : req_(req_params, std::bind(&Client::HandleReply,
                                   std::ref(*this),
                                   std::placeholders::_1,
                                   std::placeholders::_2))
      , sub_(sub_params, std::bind(&Client::HandleNotification,
                                   std::ref(*this),
                                   std::placeholders::_1))
      , router_(router) {
    SYNCER_TRY {
      T data;
      nlohmann::to_json(state_, data);
      state_[VERSION_KEY] = 0;

      req_.Request(Message());
    }
    SYNCER_CATCH_LOG("failed to construct client")
  }

  /**
   * @brief Data accessor.
   * @return a current data state.
   */
  T data() const {
    using namespace std;

    T data;
    SYNCER_TRY {
      lock_guard<mutex> _(const_cast<mutex&>(mtx_));
      nlohmann::from_json(state_, data);
    }
    SYNCER_CATCH_LOG("failed to construct data")

    return move(data);
  }

 private:
  static constexpr char const * VERSION_PATH = "/__syncer_data_version";
  static constexpr char const * VERSION_KEY = &VERSION_PATH[1];

  void HandleReply(bool success, const Message& msg) {
    using namespace nlohmann;

    if (!success) {
      SYNCER_LOG_FMT("failed to receive server's reply");
      return;
    }

    SYNCER_TRY {
      auto after = json::parse(msg.body());

      {
        std::lock_guard<std::mutex> _(mtx_);
        HandleDiff(json::diff(state_, after));
        state_ = after;
      }
    }
    SYNCER_CATCH_LOG("failed to handle reply")
  }

  void HandleNotification(const Message& msg) {
    if (msg.body_size() == 0) {
      req_.Request(Message());
      return;
    }

    SYNCER_TRY {
      auto diff = nlohmann::json::parse(msg.body());

      for (auto it = diff.begin(); it != diff.end(); ++it) {
        if ((*it)["path"] == VERSION_PATH) {
          std::lock_guard<std::mutex> _(mtx_);

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
    SYNCER_CATCH_LOG("failed to handle notification")
  }

  void HandleDiff(const nlohmann::json& diff) {
    SYNCER_TRY {
      T data;
      from_json(state_, data);

      for (auto it = diff.begin(); it != diff.end(); ++it) {
        PatchOp op;
        std::string ops = (*it)["op"];
        if (ops == "add") {
          op = PATCH_OP_ADD;
        } else if (ops == "remove") {
          op = PATCH_OP_REMOVE;
        } else if (ops == "replace") {
          op = PATCH_OP_REPLACE;
        } else {
          continue;
        }

        nlohmann::json val;
        if (op != PATCH_OP_REMOVE) {
          val = ((*it)["value"]);
        }
        router_.HandleOp(data, (*it)["path"], op, val);
      }
    }
    SYNCER_CATCH_LOG("failed to handle data diff")
  }

  PatchOpRouter<T> router_;
  nlohmann::json state_;
  std::mutex mtx_;
  Requester<Socket> req_; // to be destructed after sub_, but before others 
  Subscriber<Socket> sub_;
};

}

#endif // SYNCER_CLIENT_H_
