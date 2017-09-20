/**
 * @file config.h
 * @author Simon Prykhodko
 * @brief ZeroMQ socket configuration.
 */

#ifndef SYNCER_ZMQ_CONFIG_H_
#define SYNCER_ZMQ_CONFIG_H_

namespace syncer {
namespace zmq {

using namespace std;

/** @brief ZeroMQ socket configuration. */
struct Config {
  /** 
   * @brief Default constructor.
   * @details Part of the backend template API.
   */
  Config() {
    sndhwm = 0;
    rcvhwm = 0;
    io_threads = 1;
  }

  /** 
   * @brief Connection string constructor.
   * @details Part of the backend template API.
   * @param conn_str a ZeroMQ endpoint.
   */
  Config(const char* conn_str)
      : Config() {
    this->endpoint = conn_str;
  }

  /** @brief ZeroMQ endpoint. */
  string endpoint;

  /** @brief ZeroMQ ZMQ_SNDHWM value. */
  int sndhwm;

  /** @brief ZeroMQ ZMQ_RCVHWM value. */
  int rcvhwm;

  /** @brief Size of ZeroMQ thread pool. */
  int io_threads;
};

}
}

#endif // SYNCER_ZMQ_CONFIG_H_
