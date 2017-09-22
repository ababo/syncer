/**
 * @file publisher.h
 * @author Simon Prykhodko
 * @brief Generic publisher implementation.
 */

#ifndef SYNCER_PUBLISHER_H_
#define SYNCER_PUBLISHER_H_

#include "common.h"
#include "socket.h"

namespace syncer {

/**
 * @brief Generic publisher.
 * @details It creates and binds a PUBLISHER socket at time of construction.
 * Then it allows to publish notifications.
 */
template <typename Socket = DefaultSocket> class Publisher {
 public:
  /** @brief Alias for socket parameters. */
  using Params = typename Socket::Params;

  /** @brief Alias for socket message. */
  using Message = typename Socket::Message;

  /**
   * @brief Constructor.
   * @param params socket parameters.
   */
  Publisher(const Params& params)
      : skt_(SocketType::PUBLISHER, params) { }

  /**
   * @brief Publish a notification.
   * @param msg a message to publish.
  */
  void Publish(const Message& msg) {
    skt_.Send(msg);
  }

 private:
  Socket skt_;
};

}

#endif // SYNCER_PUBLISHER_H_
