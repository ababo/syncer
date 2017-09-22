/**
 * @file subscriber.h
 * @author Simon Prykhodko
 * @brief Generic subscriber implementation.
 */

#ifndef SYNCER_SUBSCRIBER_H_
#define SYNCER_SUBSCRIBER_H_

#include <atomic>
#include <functional>
#include <thread>

#include "common.h"
#include "socket.h"

namespace syncer {

/**
 * @brief Generic subscriber.
 * @details It creates and connects a SUBSCRIBER-socket at time of
 * construction. Then it allows to process incoming notifications via provided
 * callback. The callback will be called sequentially in a dedicated thread.
 */
template <typename Socket = DefaultSocket> class Subscriber {
 public:
  /** @brief Alias for socket parameters. */
  using Params = typename Socket::Params;

  /** @brief Alias for socket message. */
  using Message = typename Socket::Message;

  /** @brief Callback to handle notifications. */
  using Callback = function<void(const Message&)>;

  /**
   * @brief Constructor.
   * @param params socket parameters.
   * @param cb a callback for notification processing.
   */
  Subscriber(const Params& params, Callback cb)
      : exit_(false)
      , thr_(&Subscriber::Process, this, params, cb) { }

  /** @brief Destructor. */
  ~Subscriber() {
    exit_ = true;
    thr_.join();
  }

 private:
  void Process(const Params& params, Callback cb) {
    Message msg;
    msg.reserve(Message::MAX_SIZE);
    Socket skt(SocketType::SUBSCRIBER, params);

    while (!exit_) {
      if (skt.WaitToReceive()) {
        skt.Receive(msg);
        cb(msg);
      }
    }
  }

  atomic_bool exit_;
  thread thr_;
};

}

#endif // SYNCER_SUBSCRIBER_H_
