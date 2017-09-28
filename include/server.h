/**
 * @file server.h
 * @author Simon Prykhodko
 * @brief Synchronizing server implementation.
 */

#ifndef SYNCER_SERVER_H_
#define SYNCER_SERVER_H_

#include <chrono>
#include <mutex>

#include "socket.h"
#include "json.hpp"
#include "publisher.h"
#include "replier.h"
#include "timer.h"

namespace syncer {

/**
 * @brief Synchronizing server.
 * @details It creates both replier and publisher at time of construction.
 * The replier answers client requests providing a full data state. If data is
 * changed the publisher sends JSON patch (RFC 6902) which is applied on a
 * client side.
 *
 * Requirements for data type template parameter:
 *   - Must have a default constructor.
 *   - Must have `from_json` and `to_json` function overloads.
 *   - May have a move constructor (can boost performance).
 */
template <typename T, typename Socket = DefaultSocket> class Server {
 public:
  /** @brief Alias for socket parameters. */
  using Params = typename Socket::Params;

  /** @brief Alias for socket message. */
  using Message = typename Socket::Message;

  /**
   * @brief Constructor.
   * @param rep_params replier parameters.
   * @param pub_params publisher parameters.
   * @param data an initial data state.
   */
  Server(const Params& rep_params, const Params& pub_params, const T& data)
      : pub_(pub_params)
      , rep_(rep_params, std::bind(&Server::HandleRequest,
                                   std::ref(*this),
                                   std::placeholders::_1)) {
    SYNCER_TRY {
      using namespace std;

      to_json(state_, data);
      state_[VERSION_KEY] = ver_;

      {
        lock_guard<mutex> _(mtx_);
        reply_ = state_.dump();
      }

      // notify subscribers to request a full state
      timer_.Set([this] {
        pub_.Publish(Message());
      }, Socket::PUB_SUB_CONNECT_PERIOD);
    }
    SYNCER_CATCH_LOG("failed to construct server")
  }

  /**
   * @brief Constructor.
   * @details For broker-based backends (not for ZeroMQ).
   * @param params replier and publisher parameters.
   * @param data an initial data state.
   */
  Server(const Params& params, const T& data)
      : Server(params, params, data) { }

  /**
   * @brief Data accessor.
   * @return a current data state.
   */
  T data() const {
    using namespace std;

    T data;
    SYNCER_TRY {
      nlohmann::from_json(state_, data);
    }
    SYNCER_CATCH_LOG("failed to construct data")

    return move(data);
  }

  /**
   * @brief Update data state.
   * @param data a new data state.
   */
  void Update(const T& data) {
    SYNCER_TRY {
      nlohmann::json next;
      to_json(next, data);
      next[VERSION_KEY] = ver_ + 1;

      auto diff = nlohmann::json::diff(state_, next);
      if (diff.size() > 1) {
        state_ = next;
        ver_++;

        {
          std::lock_guard<std::mutex> _(mtx_);
          reply_ = state_.dump();
        }

        pub_.Publish(diff.dump());
      }
    }
    SYNCER_CATCH_LOG("failed to update server")
  }

 private:
  static constexpr char const * VERSION_KEY = "__syncer_data_version";

  Message HandleRequest(const Message&) {
    std::lock_guard<std::mutex> _(mtx_);
    return reply_;
  }

  // the field order is important!
  nlohmann::json state_;
  int ver_ = 0;
  Message reply_;
  std::mutex mtx_;
  Publisher<Socket> pub_;
  Replier<Socket> rep_;
  Timer timer_;
};

}

#endif // SYNCER_SERVER_H_
