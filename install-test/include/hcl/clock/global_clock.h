/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef INCLUDE_HCL_CLOCK_GLOBAL_CLOCK_H_
#define INCLUDE_HCL_CLOCK_GLOBAL_CLOCK_H_

#include <hcl/common/data_structures.h>
#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>
#include <hcl/common/typedefs.h>
#include <hcl/communication/rpc_lib.h>
#include <hcl/hcl_internal.h>
#include <stdint-gcc.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstdint>
#include <hcl/hcl_config.hpp>
#include <memory>
#include <string>
#include <utility>

namespace bip = boost::interprocess;

namespace hcl {
class global_clock {
 private:
  typedef std::chrono::high_resolution_clock::time_point chrono_time;
  chrono_time *start;
  bool is_server;
  bip::interprocess_mutex *mutex;
  really_long memory_allocated;
  int num_servers;
  uint16_t my_server;
  bip::managed_mapped_file segment;
  std::string name, func_prefix;
  uint16_t port;
  bool server_on_node;
  CharStruct backed_file;

 public:
  /*
   * Destructor removes shared memory from the server
   */
  ~global_clock();

  global_clock(std::string name_ = "TEST_GLOBAL_CLOCK",
               uint16_t _port = HCL_CONF->RPC_PORT);
  chrono_time *data();
  void lock();
  void unlock();
  /*
   * GetTime() returns the time on the server
   */
  HTime GetTime();
  /*
   * GetTimeServer() returns the time on the requested server using RPC calls,
   * or the local time if the server requested is the current client server
   */
  HTime GetTimeServer(uint16_t &server);

  /*
   * GetTime() returns the time locally within a node using chrono
   * high_resolution_clock
   */
  HTime LocalGetTime();

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE1(LocalGetTime)
#endif
};

}  // namespace hcl

#endif  // INCLUDE_HCL_CLOCK_GLOBAL_CLOCK_H_
