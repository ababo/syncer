/** ZeroMQ socket wrapper. */

#ifndef SYNCER_ZMQ_SOCKET_H_
#define SYNCER_ZMQ_SOCKET_H_

#include <zmq.h>

#include "config.h"

namespace syncer {
namespace zmq {

/**
 * ZeroMQ socket.
 * Depending on type binds or connects at time of construction.
 */
class Socket {
 public:
  /** Maximal size of transmitted message. */
  static const int MAX_MSG_SIZE = 1024 * 1024;

  /** Default waiting timeout in milliseconds. */
  static const int WAIT_TIMEOUT = 100;

  /**
   * Constructor.
   * @param type ZeroMQ socket type.
   * @param conf socket configuration.
   */
  Socket(int type, const Config& conf) {
    ctx_ = CreateContext(conf);
    skt_ = CreateSocket(ctx_, type, conf);
  }

  /** Destructor. */
  ~Socket() {
    zmq_ctx_destroy(ctx_);
  }

  /** Raw ZeroMQ context. */
  void* context() const { return ctx_; }

  /** Raw ZeroMQ socket. */
  void* socket() const { return skt_; }

  /**
   * Send message.
   * @param msg a message to send.
   * @param flags ZeroMQ sending flags.
   * @return true if succeeded.
   */
  bool Send(const Message& msg, int flags = 0) {
    if (zmq_send(skt_, msg.c_str(), msg.size(), flags) == -1) {
      SYNCER_LOG_FMTE("failed to send from ZMQ socket");
      return false;
    }
    return true;
  }

  /**
   * Receive message.
   * @param msg[out] a message to receive.
   * @param flags ZeroMQ receiving flags.
   * @return true if succeeded.
   */
  bool Receive(Message& msg, int flags = 0) {
    msg.resize(MAX_MSG_SIZE);

    int num = zmq_recv(skt_, &msg[0], MAX_MSG_SIZE, flags);
    if (num != -1) {
      msg.resize(num);
      return true;
    } else {
      SYNCER_LOG_FMTE("Failed to receive from ZMQ socket");
      return false;
    }
  }

  /**
   * Wait for a message to arrive.
   * @param timeout a timeout in milliseconds (-1 to wait forever).
   * @return true if a message has arrived.
   */
  bool WaitToReceive(int timeout = WAIT_TIMEOUT) {
    zmq_pollitem_t item;
    item.socket = skt_;
    item.events = ZMQ_POLLIN;

    auto num = zmq_poll(&item, 1, timeout);
    if (num == -1) {
      SYNCER_LOG_FMTE("Failed to poll ZMQ socket");
      return false;
    }

    return num > 0;
  }

 private:
  static void* CreateContext(const Config& conf) {
    auto ctx = zmq_ctx_new();
    if (ctx == nullptr) {
      SYNCER_LOG_FMTE("failed to create ZMQ context");
      return nullptr;
    }

    if (zmq_ctx_set(ctx, ZMQ_IO_THREADS, conf.io_threads) == -1) {
      SYNCER_LOG_FMTE("failed to set ZMQ_IO_THREADS for ZMQ context");
    }

    return ctx;
  }

  static void* CreateSocket(void* ctx, int type, const Config& conf) {
    void* skt = zmq_socket(ctx, type);
    if (skt == nullptr) {
      SYNCER_LOG_FMTE("failed to create ZMQ socket");
      return nullptr;
    }

    int val = conf.sndhwm;
    if (zmq_setsockopt(skt, ZMQ_SNDHWM, &val, sizeof(val)) == -1) {
      SYNCER_LOG_FMTE("failed to set ZMQ_SNDHWM for ZMQ socket");
    }

    val = conf.rcvhwm;
    if (zmq_setsockopt(skt, ZMQ_RCVHWM, &val, sizeof(val)) == -1) {
      SYNCER_LOG_FMTE("failed to set ZMQ_RCVHWM for ZMQ socket");
    }

    if (type == ZMQ_REP || type == ZMQ_PUB) {
      if (zmq_bind(skt, conf.endpoint.c_str()) == -1) {
        SYNCER_LOG_FMTE("failed to bind ZMQ socket");
      }
    } else {
      if (type == ZMQ_SUB
            && zmq_setsockopt(skt, ZMQ_SUBSCRIBE, nullptr, 0) == -1) {
        SYNCER_LOG_FMTE("failed to set ZMQ_SUBSCRIBE for ZMQ socket");
      }
      if (zmq_connect(skt, conf.endpoint.c_str()) == -1) {
        SYNCER_LOG_FMTE("failed to connect ZMQ socket");
      }
    }

    return skt;
  }

  void* ctx_;
  void* skt_;
};

}
}

#endif // SYNCER_ZMQ_SOCKET_H_
