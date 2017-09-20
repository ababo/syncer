/**
 * @file requester.h
 * @author Simon Prykhodko
 * @brief ZeroMQ requester implementation.
 */

#ifndef SYNCER_ZMQ_REQUESTER_H_
#define SYNCER_ZMQ_REQUESTER_H_

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <thread>

#include "../common.h"
#include "config.h"
#include "socket.h"

namespace syncer {
namespace zmq {

using namespace std;

/**
 * @brief ZeroMQ requester.
 * @details It creates and connects a REQ-socket at time of construction. Then
 * it allows to send requests, handling the corresponding replies via provided
 * callback. The callback will be called sequentially in a dedicated thread.
 */
class Requester {
 public:
  /** @brief Default request waiting timeout in milliseconds. */
  static const int WAIT_TIMEOUT = 1000;

  /**
   * @brief Constructor.
   * @details Part of the backend template API.
   * @param conf a socket configuration.
   * @param cb a callback for reply processing.
   */
  Requester(const Config& conf, ReplyCallback cb)
      : exit_(false)
      , thr_(&Requester::Process, this, conf, cb) { }

  /**
   * @brief Destructor.
   * @details Part of the backend template API.
   */
  ~Requester() {
    exit_ = true;
    cv_.notify_one();
    thr_.join();
  }

  /**
   * @brief Send a request to handle reply via provided callback.
   * @details If the connected replier doesn't respond before expiring the
   * specified timeout or some another error occurs, the provided callback is
   * triggered to indicate failure. Part of the backend template API.
   * @param req a request message.
   * @param timeout a timeout in milliseconds.
  */
  void Request(const Message& req, int timeout = WAIT_TIMEOUT) {
    {
      lock_guard<mutex> _(mtx_);
      req_ = req, timeout_ = timeout;
    }
    cv_.notify_one();
  }

 private:
  void Process(const Config& conf, ReplyCallback cb) {
    Message msg;
    msg.reserve(Socket::MAX_MSG_SIZE);
    Socket skt(ZMQ_REQ, conf);

    for (;;) {
      unique_lock<mutex> lock(mtx_);
      cv_.wait(lock, [this] { return exit_ || timeout_ > 0; });

      if (exit_) {
        return;
      }

      skt.Send(req_);

      bool success = false;
      for (int waited = 0; !exit_ && waited < timeout_; ) {
        int tmp = Socket::WAIT_TIMEOUT; // to avoid linker error
        int timeout = min(tmp, timeout_ - waited);

        if (skt.WaitToReceive(timeout)) {
          skt.Receive(msg);
          success = true;
          break;
        }

        waited += timeout;
      }

      if (exit_) {
        return;
      }

      timeout_ = 0;

      lock.unlock();

      cb(success, msg);
    }
  }

  atomic_bool exit_;
  thread thr_;
  int timeout_;
  Message req_;
  mutex mtx_;
  condition_variable cv_;
};

}
}

#endif // SYNCER_ZMQ_REQUESTER_H_
