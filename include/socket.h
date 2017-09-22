/**
 * @file socket.h
 * @author Simon Prykhodko
 * @brief Logic for inclusion of different socket implementations.
 */

#ifndef SYNCER_SOCKET_H_
#define SYNCER_SOCKET_H_

/** @brief Include ZeroMQ socket. */
#ifdef SYNCER_ZMQ
#include "zmq_socket.h"
#endif

/**
 * @brief Avoids assigning default socket if defined.
 * @details By default ZeroMQ socket is assigned as a default one, but user
 * can change that by defining this macro. In that case a custom default
 * socket should be provided.
 */
#ifndef SYNCER_CUSTOM_DEFAULT_SOCKET
#include "zmq_socket.h"
namespace syncer {
/** Default socket type. */
using DefaultSocket = ZMQSocket;
}
#endif // SYNCER_CUSTOM_DEFAULT_SOCKET

#endif // SYNCER_SOCKET_H_
