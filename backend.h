/** Backend inclusion logic. */

#ifndef SYNCER_BACKEND_H_
#define SYNCER_BACKEND_H_

/** Causes inclusion of ZeroMQ backend if defined. */
#ifdef SYNCER_ZMQ
#include "zmq/backend.h"
#endif

/**
 * Default backend for templates instantiation.
 * By default it's ZeroMQ, but this can be changed by user.
 */
#ifndef SYNCER_DEFAULT_BACKEND
#include "zmq/backend.h"
#define SYNCER_DEFAULT_BACKEND zmq::Backend
#endif

#endif // SYNCER_BACKEND_H_
