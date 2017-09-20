/**
 * @file syncer/backend.h
 * @author Simon Prykhodko
 * @brief Backend inclusion logic.
 */

#ifndef SYNCER_BACKEND_H_
#define SYNCER_BACKEND_H_

/** @brief Causes inclusion of ZeroMQ backend if defined. */
#ifdef SYNCER_ZMQ
#include "zmq/backend.h"
#endif

/**
 * @brief Default backend for templates instantiation.
 * @details By default it's ZeroMQ, but this can be changed by user.
 */

/**
 * @brief Avoids assigning default backend if defined.
 * @details By default ZeroMQ backend is assigned as a default one, but user
 * can change that by defining this macro. In that case a custom default
 * backend should be provided.
 */
#ifndef SYNCER_CUSTOM_DEFAULT_BACKEND
#include "zmq/backend.h"
namespace syncer{
using DefaultBackend = zmq::Backend;
}
#endif // SYNCER_CUSTOM_DEFAULT_BACKEND

#endif // SYNCER_BACKEND_H_
