/** ZeroMQ publisher implementation. */

#ifndef SYNCER_ZMQ_PUBLISHER_H_
#define SYNCER_ZMQ_PUBLISHER_H_

#include "../common.h"
#include "config.h"

namespace syncer {
namespace zmq {

/** 
 * ZeroMQ publisher.
 * Binds its socket at time of construction and allows to publish arbitrary
 * data via PUB-socket.
 */
class Publisher {
 public:
  /**
   * Constructor.
   * Binds an internal PUB-socket. 
   */
  Publisher(const Config& config);

  /** Destructor. */
  ~Publisher();

  /**
   * Publish message.
   * @param msg a message to publish.
   * @param more true if msg is partial and further parts are to follow.
  */
  void Publish(const Message& msg);

 private:

};

}
}

#endif // SYNCER_ZMQ_PUBLISHER_H_
