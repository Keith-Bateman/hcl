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

#ifndef INCLUDE_HCL_CONCURRENT_QUEUE_H_
#define INCLUDE_HCL_CONCURRENT_QUEUE_H_
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
/**
 * Include Headers
 */

#include <hcl/common/container.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>
#include <hcl/communication/rpc_factory.h>
#include <hcl/communication/rpc_lib.h>
/** MPI Headers**/
#include <mpi.h>

/** RPC Lib Headers**/
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <rpc/server.h>
#endif

/** Thallium Headers **/
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
#include <thallium.hpp>
#endif

/** Boost Headers **/
#include <boost/algorithm/string.hpp>
#include <boost/lockfree/queue.hpp>
/** Standard C++ Headers**/
#include <float.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/*This file contains the class that implements a distributed queue, distributed
 * across all servers. A client can choose any of the servers and make an RPC
 * call to perform queue operations.*/

namespace hcl {

template <class ValueT>
class concurrent_queue : public Container {
 public:
  typedef boost::lockfree::queue<ValueT> queue_type;

 private:
  queue_type *queue;

 public:
  ~concurrent_queue();

  void construct_shared_memory() override;
  void open_shared_memory() override;
  void bind_functions() override;

  concurrent_queue(CharStruct name_ = "TEST_CONCURRENT_QUEUE",
                   uint16_t port = 0);

  queue_type *data();

  bool LocalPush(ValueT &v);
  std::pair<bool, ValueT> LocalPop();

  bool Push(uint64_t &s, ValueT &v);

  std::pair<bool, ValueT> Pop(uint64_t &s);
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalPush, (v), ValueT &v)
  THALLIUM_DEFINE1(LocalPop)
#endif
};
}  // namespace hcl

#endif
