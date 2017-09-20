/**
 * @file zmq/backend.h
 * @author Simon Prykhodko
 * @brief Set of communication primitives based on ZeroMQ.
 */

#ifndef SYNCER_ZMQ_BACKEND_H_
#define SYNCER_ZMQ_BACKEND_H_

#include "config.h"
#include "publisher.h"
#include "replier.h"
#include "requester.h"
#include "subscriber.h"

namespace syncer {
/** @brief ZeroMQ backend stuff. */
namespace zmq {

/** 
 * @brief ZeroMQ backend.
 * @details It references set of communication primitives based on ZeroMQ.
 */
class Backend {
 public:
  using Config = zmq::Config;
  using Publisher = zmq::Publisher;
  using Replier = zmq::Replier;
  using Requester = zmq::Requester;
  using Subscriber = zmq::Subscriber;
};

}
}

#endif // SYNCER_ZMQ_BACKEND_H_
