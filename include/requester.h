/**
 * @file requester.h
 * @author Simon Prykhodko
 * @brief Generic requester implementation.
 */

#ifndef SYNCER_REQUESTER_H_
#define SYNCER_REQUESTER_H_

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

#include "common.h"
#include "socket.h"

namespace syncer {

/**
 * @brief Generic requester.
 * @details It creates and connects a REQUESTER-socket at time of construction.
 * Then it allows to send requests, handling the corresponding replies via
 * provided callback. The callback will be called sequentially in a dedicated
 * thread.
 */
template <typename Socket = DefaultSocket> class Requester {
 public:
  /** @brief Alias for socket parameters. */
  using Params = typename Socket::Params;

  /** @brief Alias for socket message. */
  using Message = typename Socket::Message;

  /** @brief Callback to handle replies. */
  using Callback = std::function<void(bool, const Message&)>;

  /** @brief Default request waiting timeout in milliseconds. */
  static const int WAIT_TIMEOUT = 1000;

  /**
   * @brief Constructor.
   * @param params socket parameters.
   * @param cb a callback for reply processing.
   */
  Requester(const Params& params, Callback cb)
      : exit_(false)
      , thr_(&Requester::Process, this, params, cb) { }

  /** @brief Destructor. */
  ~Requester() {
    exit_ = true;
    cv_.notify_one();
    thr_.join();
  }

  /**
   * @brief Send a request to handle reply via provided callback.
   * @details If the connected replier doesn't respond before expiring the
   * specified timeout or some another error occurs, the provided callback is
   * triggered to indicate failure.
   * @param req a request message.
   * @param timeout a timeout in milliseconds.
  */
  void Request(const Message& req, int timeout = WAIT_TIMEOUT) {
    {
      std::lock_guard<std::mutex> _(mtx_);
      req_ = req, timeout_ = timeout;
    }
    cv_.notify_one();
  }

 private:
  void Process(const Params& params, Callback cb) {
    using namespace std;

    typename Socket::Message msg;
    msg.reserve(Message::MAX_SIZE);
    Socket skt(SocketType::REQUESTER, params);

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

  std::atomic_bool exit_;
  std::thread thr_;
  int timeout_;
  Message req_;
  std::mutex mtx_;
  std::condition_variable cv_;
};

}

#endif // SYNCER_REQUESTER_H_
