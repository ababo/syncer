/** ZeroMQ socket. */

#ifndef SYNCER_ZMQ_SOCKET_H_
#define SYNCER_ZMQ_SOCKET_H_

#include "config.h"

namespace syncer {
namespace zmq {

/** 
 * ZeroMQ socket.
 * Depending on type binds or connects at time of construction.
 */
class Socket {
 public:
  /**
   * Constructor.
   * @param type ZeroMQ socket type.
   * @param conf socket configuration.
   */
  Socket(int type, const Config& conf);

  /** Destructor. */
  ~Socket();

  /**
   * Send message.
   * @param msg a message to send.
   * @param flags ZeroMQ sending flags.
   * @return true if succeeded.
   */
  bool Send(const Message& msg, int flags);

  /**
   * Receive message.
   * @param msg[out] a message to receive.
   * @param flags ZeroMQ receiving flags.
   * @return true if succeeded.
   */
  bool Receive(Message& msg, int flags);

  /**
   * Wait for event to fire.
   * @param timeout a timeout in milliseconds (-1 to wait forever).
   * @param mask of ZeroMQ events to fire.
   * @param revents[out] ZeroMQ events fired (optional).
   * @return true if succeeded.
   */
  bool Poll(int timeout, int events, int* revents = NULL);

 private:
  void* ctx_;
  void* skt_;
};

}
}

#endif // SYNCER_ZMQ_SOCKET_H_
