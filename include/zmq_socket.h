/**
 * @file zmq_socket.h
 * @author Simon Prykhodko
 * @brief ZeroMQ socket implementation.
 */

#ifndef SYNCER_ZMQ_SOCKET_H_
#define SYNCER_ZMQ_SOCKET_H_

#include <string>
#include <zmq.h>

#include "common.h"

namespace syncer {

using namespace std;

/**
 * @brief ZeroMQ socket.
 * @details Depending on type binds or connects at time of construction.
 */
class ZMQSocket {
 public:

  /** @brief Socket parameters. */
  struct Params {
    /**
     * @brief Default constructor.
     * @details Part of the socket template API.
     */
    Params() {
      sndhwm = 0;
      rcvhwm = 0;
      io_threads = 1;
    }

    /**
     * @brief Connection string constructor.
     * @details Part of the socket template API.
     * @param conn_str a ZeroMQ endpoint.
     */
    Params(const char* conn_str)
        : Params() {
      this->endpoint = conn_str;
    }

    /** @brief ZeroMQ endpoint. */
    string endpoint;

    /** @brief ZeroMQ ZMQ_SNDHWM value. */
    int sndhwm;

    /** @brief ZeroMQ ZMQ_RCVHWM value. */
    int rcvhwm;

    /** @brief Size of ZeroMQ thread pool. */
    int io_threads;
  };

  /**
   * @brief Constructor.
   * @details Part of the socket template API.
   * @param type a socket type.
   * @param params socket parameters.
   */
  ZMQSocket(SocketType type, const Params& params) {
    ctx_ = CreateContext(params);
    skt_ = CreateSocket(ctx_, GetZMQSocketType(type), params);
  }

  /**
   * @brief Destructor.
   * @details Part of the socket template API.
   */
  ~ZMQSocket() {
    zmq_close(skt_);
    zmq_ctx_destroy(ctx_);
  }

  /** @brief Raw ZeroMQ context. */
  void* raw_context() const { return ctx_; }

  /** @brief Raw ZeroMQ socket. */
  void* raw_socket() const { return skt_; }

  /**
   * @brief Send message.
   * @details Part of the socket template API.
   * @param msg a message to send.
   * @return `true` if succeeded.
   */
  bool Send(const Message& msg) {
    if (zmq_send(skt_, msg.c_str(), msg.size(), 0) == -1) {
      SYNCER_LOG_FMTE("failed to send from ZMQ socket");
      return false;
    }
    return true;
  }

  /**
   * @brief Receive message.
   * @details Part of the socket template API.
   * @param msg a message to receive.
   * @return `true` if succeeded.
   */
  bool Receive(Message& msg) {
    msg.resize(MAX_MSG_SIZE);

    int num = zmq_recv(skt_, &msg[0], MAX_MSG_SIZE, 0);
    if (num != -1) {
      msg.resize(num);
      return true;
    } else {
      SYNCER_LOG_FMTE("failed to receive from ZMQ socket");
      return false;
    }
  }

  /**
   * @brief Wait for a message to arrive.
   * @details Part of the socket template API.
   * @param timeout a timeout in milliseconds (-1 to wait forever).
   * @return `true` if a message has arrived.
   */
  bool WaitToReceive(int timeout = SOCKET_WAIT_TIMEOUT) {
    zmq_pollitem_t item;
    item.socket = skt_;
    item.events = ZMQ_POLLIN;

    auto num = zmq_poll(&item, 1, timeout);
    if (num == -1) {
      SYNCER_LOG_FMTE("failed to poll ZMQ socket");
      return false;
    }

    return num > 0;
  }

 private:
  static int GetZMQSocketType(SocketType type) {
    switch (type) {
    case SocketType::PUBLISHER: return ZMQ_PUB;
    case SocketType::SUBSCRIBER: return ZMQ_SUB;
    case SocketType::REPLIER: return ZMQ_REP;
    case SocketType::REQUESTER: return ZMQ_REQ;
    default:
      SYNCER_LOG_FMT("unknown socket type");
      return -1;
    }
  }

  static void* CreateContext(const Params& params) {
    auto ctx = zmq_ctx_new();
    if (ctx == nullptr) {
      SYNCER_LOG_FMTE("failed to create ZMQ context");
      return nullptr;
    }

    if (zmq_ctx_set(ctx, ZMQ_IO_THREADS, params.io_threads) == -1) {
      SYNCER_LOG_FMTE("failed to set ZMQ_IO_THREADS for ZMQ context");
    }

    return ctx;
  }

  static void* CreateSocket(void* ctx, int type, const Params& params) {
    void* skt = zmq_socket(ctx, type);
    if (skt == nullptr) {
      SYNCER_LOG_FMTE("failed to create ZMQ socket");
      return nullptr;
    }

    int val = params.sndhwm;
    if (zmq_setsockopt(skt, ZMQ_SNDHWM, &val, sizeof(val)) == -1) {
      SYNCER_LOG_FMTE("failed to set ZMQ_SNDHWM for ZMQ socket");
    }

    val = params.rcvhwm;
    if (zmq_setsockopt(skt, ZMQ_RCVHWM, &val, sizeof(val)) == -1) {
      SYNCER_LOG_FMTE("failed to set ZMQ_RCVHWM for ZMQ socket");
    }

    if (type == ZMQ_REP || type == ZMQ_PUB) {
      if (zmq_bind(skt, params.endpoint.c_str()) == -1) {
        SYNCER_LOG_FMTE("failed to bind ZMQ socket");
      }
    } else {
      if (type == ZMQ_SUB
            && zmq_setsockopt(skt, ZMQ_SUBSCRIBE, nullptr, 0) == -1) {
        SYNCER_LOG_FMTE("failed to set ZMQ_SUBSCRIBE for ZMQ socket");
      }
      if (zmq_connect(skt, params.endpoint.c_str()) == -1) {
        SYNCER_LOG_FMTE("failed to connect ZMQ socket");
      }
    }

    return skt;
  }

  void* ctx_;
  void* skt_;
};

}

#endif // SYNCER_ZMQ_SOCKET_H_
