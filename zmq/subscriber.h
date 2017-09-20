/**
 * @file subscriber.h
 * @author Simon Prykhodko
 * @brief ZeroMQ subscriber implementation.
 */

#ifndef SYNCER_ZMQ_SUBSCRIBER_H_
#define SYNCER_ZMQ_SUBSCRIBER_H_

#include <atomic>
#include <thread>

#include "../common.h"
#include "config.h"
#include "socket.h"

namespace syncer {
namespace zmq {

/**
 * @brief ZeroMQ subscriber.
 * @details It creates and connects a SUB-socket at time of construction. Then
 * it allows to process incoming notifications via provided callback. The
 * callback will be called sequentially in a dedicated thread.
 */
class Subscriber {
 public:
  /**
   * @brief Constructor.
   * @details Part of the backend template API.
   * @param conf a socket configuration.
   * @param cb a callback for notification processing.
   */
  Subscriber(const Config& conf, NotificationCallback cb)
      : exit_(false)
      , thr_(&Subscriber::Process, this, conf, cb) { }

  /**
   * @brief Destructor.
   * @details Part of the backend template API.
   */
  ~Subscriber() {
    exit_ = true;
    thr_.join();
  }

 private:
  void Process(const Config& conf, NotificationCallback cb) {
    Message msg;
    msg.reserve(Socket::MAX_MSG_SIZE);
    Socket skt(ZMQ_SUB, conf);

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
}

#endif // SYNCER_ZMQ_SUBSCRIBER_H_
