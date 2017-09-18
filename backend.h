#ifndef SYNCER_BACKEND_H_
#define SYNCER_BACKEND_H_

#ifdef SYNCER_ZMQ
#include "zmq/backend.h"
#endif

#ifndef SYNCER_DEFAULT_BACKEND
#include "zmq/backend.h"
#define SYNCER_DEFAULT_BACKEND zmq::Backend
#endif

#endif // SYNCER_BACKEND_H_
