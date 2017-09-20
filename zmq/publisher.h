/**
 * @file publisher.h
 * @author Simon Prykhodko
 * @brief ZeroMQ publisher implementation.
 */

#ifndef SYNCER_ZMQ_PUBLISHER_H_
#define SYNCER_ZMQ_PUBLISHER_H_

#include "../common.h"
#include "config.h"
#include "socket.h"

namespace syncer {
namespace zmq {

/**
 * @brief ZeroMQ publisher.
 * @details It creates and binds a PUB-socket at time of construction. Then it
 * allows to publish notifications.
 */
class Publisher {
 public:
  /**
   * @brief Constructor.
   * @details Part of the backend template API.
   */
  Publisher(const Config& config)
      : skt_(ZMQ_PUB, config) { }

  /**
   * @brief Publish a notification.
   * @details Part of the backend template API.
   * @param msg a message to publish.
  */
  void Publish(const Message& msg) {
    skt_.Send(msg);
  }

 private:
  Socket skt_;
};

}
}

#endif // SYNCER_ZMQ_PUBLISHER_H_
