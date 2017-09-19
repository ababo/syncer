/** ZeroMQ replier implementation. */

#ifndef SYNCER_ZMQ_REPLIER_H_
#define SYNCER_ZMQ_REPLIER_H_

#include <atomic>
#include <thread>

#include "../common.h"
#include "config.h"
#include "socket.h"

namespace syncer {
namespace zmq {

/**
 * ZeroMQ replier.
 * It creates and binds a REP-socket at time of construction. Then it allows to
 * process incoming requests via provided callback, sending the corresponding
 * replies. The callback will be called sequentially in a dedicated thread.
 */
class Replier {
 public:
  /**
   * Constructor.
   * Part of the backend template API.
   * @param conf a socket configuration.
   * @param cb a callback for request processing.
   */
  Replier(const Config& conf, RequestCallback cb)
      : exit_(false)
      , thr_(&Replier::Process, this, conf, cb) { }

  /**
   * Destructor.
   * Part of the backend template API.
   */
  ~Replier() {
    exit_ = true;
    thr_.join();
  }

 private:
  void Process(const Config& conf, RequestCallback cb) {
    Message msg;
    msg.reserve(Socket::MAX_MSG_SIZE);
    Socket skt(ZMQ_REP, conf);

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
}

#endif // SYNCER_ZMQ_REPLIER_H_
