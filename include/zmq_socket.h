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

/**
 * @brief ZeroMQ socket.
 * @details Depending on type binds or connects at time of construction.
 */
class ZMQSocket {
 public:
  /** @brief Socket parameters. */
  struct Params {
    /**
     * @brief Constructor.
     * @details Part of the socket template API.
     * @param conn_str a connection string.
     * @param subject a subscriber's subject.
     */
    Params(const char* conn_str, const char* subject = nullptr) {
      this->conn_str = conn_str;
      if (subject != nullptr) {
        this->subject = subject;
      }
    }

    /**
     * @brief ZeroMQ endpoint.
     * @details Part of the socket template API.
     */
    std::string conn_str;

    /**
     * @brief ZeroMQ subscriber's filter.
     * @details Part of the socket template API.
     */
    std::string subject;

    /** @brief ZeroMQ ZMQ_SNDHWM value. */
    int sndhwm = 0;

    /** @brief ZeroMQ ZMQ_RCVHWM value. */
    int rcvhwm = 0;

    /** @brief Size of ZeroMQ thread pool. */
    int io_threads = 1;

    /** @brief ZeroMQ linger period in milliseconds. */
    int linger = 0;
  };

  /**
   * @brief Socket message.
   * @details Part of the socket template API.
   */
  class Message {
   public:
    /**
     * @brief Maximal message capacity.
     * @details Part of the socket template API.
     */
    static const int MAX_SIZE = 1024 * 1024;

    /**
     * @brief Constructor.
     * @details Part of the socket template API.
     */
    Message() {
      set("");
    }

    /**
     * @brief Constructor.
     * @details Part of the socket template API.
     * @param body a message body.
     */
    Message(const std::string& body) {
      set(body);
    }

    /**
     * @brief Constructor.
     * @details Part of the socket template API.
     * @param subj a message subject.
     * @param body a message body.
     */
    Message(const std::string& subj, const std::string& body) {
      set(subj, body);
    }

    /**
     * @brief Reserve a message capacity.
     * @details Part of the socket template API.
     * @param size a message capacity.
     */
    void reserve(size_t size) {
      data_.reserve(size);
    }

    /**
     * @brief Set message content.
     * @details Part of the socket template API.
     * @param body a message body.
     */
    void set(const std::string& body) {
      data_.push_back(0);
      data_ += body;
      ssize_ = 0;
    }

    /**
     * @brief Set message content.
     * @details Part of the socket template API.
     * @param subj a message subject.
     * @param body a message body.
     */
    void set(const std::string& subj, const std::string& body) {
      data_ = subj;
      data_.push_back(0);
      data_ += body;
      ssize_ = subj.size();
    }

    /**
     * @brief Message subject getter.
     * @details Part of the socket template API.
     */
    const char* subject() const {
      return data_.c_str();
    }

    /**
     * @brief Message subject size.
     * @details Part of the socket template API.
     */
    size_t subject_size() const {
      return ssize_;
    }

    /**
     * @brief Message body getter.
     * @details Part of the socket template API.
     */
    const char* body() const {
      return &data_.c_str()[ssize_ + 1];
    }

    /**
     * @brief Message body size.
     * @details Part of the socket template API.
     */
    size_t body_size() const {
      return data_.size() - ssize_ - 1;
    }

   private:
    friend class ZMQSocket;

    std::string data_;
    int ssize_ = 0;
  };

  /**
   * @brief Waiting timeout in milliseconds.
   * @details Part of the socket template API.
   */
  static const int WAIT_TIMEOUT = 100;

  /**
   * @brief Publish/subscribe connection period in milliseconds.
   * @details This is an empirically obtained maximum of connection time
   * between publisher and subscribers. Part of the socket template API.
   */
  static const int PUB_SUB_CONNECT_PERIOD = 250;

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
   */
  void Send(const Message& msg) {
    if (zmq_send(skt_, msg.data_.c_str(), msg.data_.size(), 0) == -1) {
      SYNCER_LOG_ERROR("failed to send from ZMQ socket");
    }
  }

  /**
   * @brief Receive message.
   * @details Part of the socket template API.
   * @param msg a message to receive.
   */
  void Receive(Message& msg) {
    msg.data_.resize(Message::MAX_SIZE);

    int num = zmq_recv(skt_, &msg.data_[0], Message::MAX_SIZE, 0);
    if (num != -1) {
      msg.data_.resize(num);
      msg.ssize_ = strlen(msg.data_.c_str());
    } else {
      SYNCER_LOG_ERROR("failed to receive from ZMQ socket");
    }
  }

  /**
   * @brief Wait for a message to arrive.
   * @details Part of the socket template API.
   * @param timeout a timeout in milliseconds (-1 to wait forever).
   * @return `true` if a message has arrived.
   */
  bool WaitToReceive(int timeout = WAIT_TIMEOUT) {
    zmq_pollitem_t item;
    item.socket = skt_;
    item.events = ZMQ_POLLIN;

    auto num = zmq_poll(&item, 1, timeout);
    if (num == -1) {
      SYNCER_LOG_ERROR("failed to poll ZMQ socket");
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
      SYNCER_LOG_ERROR("failed to create ZMQ context");
      return nullptr;
    }

    if (zmq_ctx_set(ctx, ZMQ_IO_THREADS, params.io_threads) == -1) {
      SYNCER_LOG_ERROR("failed to set ZMQ_IO_THREADS for ZMQ context");
    }

    return ctx;
  }

  static void* CreateSocket(void* ctx, int type, const Params& params) {
    void* skt = zmq_socket(ctx, type);
    if (skt == nullptr) {
      SYNCER_LOG_ERROR("failed to create ZMQ socket");
      return nullptr;
    }

    int val = params.sndhwm;
    if (zmq_setsockopt(skt, ZMQ_SNDHWM, &val, sizeof(val)) == -1) {
      SYNCER_LOG_ERROR("failed to set ZMQ_SNDHWM for ZMQ socket");
    }

    val = params.rcvhwm;
    if (zmq_setsockopt(skt, ZMQ_RCVHWM, &val, sizeof(val)) == -1) {
      SYNCER_LOG_ERROR("failed to set ZMQ_RCVHWM for ZMQ socket");
    }

    val = params.linger;
    if (zmq_setsockopt(skt, ZMQ_LINGER, &val, sizeof(val)) == -1) {
      SYNCER_LOG_ERROR("failed to set ZMQ_LINGER for ZMQ socket");
    }

    if (type == ZMQ_REP || type == ZMQ_PUB) {
      if (zmq_bind(skt, params.conn_str.c_str()) == -1) {
        SYNCER_LOG_ERROR("failed to bind ZMQ socket");
      }
    } else {
      if (type == ZMQ_SUB && zmq_setsockopt(skt, ZMQ_SUBSCRIBE,
                                            params.subject.c_str(),
                                            params.subject.size()) == -1) {
        SYNCER_LOG_ERROR("failed to set ZMQ_SUBSCRIBE for ZMQ socket");
      }
      if (zmq_connect(skt, params.conn_str.c_str()) == -1) {
        SYNCER_LOG_ERROR("failed to connect ZMQ socket");
      }
    }

    return skt;
  }

  void* ctx_;
  void* skt_;
};

}

#endif // SYNCER_ZMQ_SOCKET_H_
