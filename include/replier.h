/**
 * @file replier.h
 * @author Simon Prykhodko
 * @brief Generic replier implementation.
 */

#ifndef SYNCER_REPLIER_H_
#define SYNCER_REPLIER_H_

#include <atomic>
#include <functional>
#include <thread>

#include "common.h"
#include "socket.h"

namespace syncer {

/**
 * @brief Generic replier.
 * @details It creates and binds a REPLIER-socket at time of construction.
 * Then it allows to process incoming requests via provided callback, sending
 * the corresponding replies. The callback will be called sequentially in a
 * dedicated thread.
 */
template <typename Socket = DefaultSocket> class Replier {
 public:
  /** @brief Alias for socket parameters. */
  using Params = typename Socket::Params;

  /** @brief Alias for socket message. */
  using Message = typename Socket::Message;

  /** @brief Callback to handle requests. */
  using Callback = function<Message(const Message&)>;

  /**
   * @brief Constructor.
   * @param params socket parameters.
   * @param cb a callback for request processing.
   */
  Replier(const Params& params, Callback cb)
      : exit_(false)
      , thr_(&Replier::Process, this, params, cb) { }

  /** @brief Destructor. */
  ~Replier() {
    exit_ = true;
    thr_.join();
  }

 private:
  void Process(const Params& params, Callback cb) {
    Message msg;
    msg.reserve(Message::MAX_SIZE);
    Socket skt(SocketType::REPLIER, params);

    while (!exit_) {
      if (skt.WaitToReceive()) {
        skt.Receive(msg);
        skt.Send(cb(msg));
      }
    }
  }

  atomic_bool exit_;
  thread thr_;
};

}

#endif // SYNCER_REPLIER_H_
