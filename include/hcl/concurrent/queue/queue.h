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

#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>
#include <hcl/communication/rpc_factory.h>
#include <hcl/communication/rpc_lib.h>
/** MPI Headers**/
#include <mpi.h>

/** Thallium Headers **/
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
#include <thallium.hpp>
#endif

/** Boost Headers **/
#include <boost/algorithm/string.hpp>
#include <boost/lockfree/queue.hpp>
/** Standard C++ Headers**/
#include <float.h>
#include <hcl/common/container.h>

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
class concurrent_queue : public container {
 public:
  typedef boost::lockfree::queue<ValueT> queue_type;

 private:
  queue_type *queue;

 public:
  ~concurrent_queue() {
    if (queue != nullptr) delete queue;
  }

  void construct_shared_memory() override {}
  void open_shared_memory() override {}
  void bind_functions() override {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {

        std::function<void(const tl::request &, ValueT &)> pushFunc(
            std::bind(&concurrent_queue<ValueT>::ThalliumLocalPush, this,
                      std::placeholders::_1, std::placeholders::_2));
        std::function<void(const tl::request &)> popFunc(
            std::bind(&concurrent_queue<ValueT>::ThalliumLocalPop, this,
                      std::placeholders::_1));

        rpc->bind(func_prefix + "_Push", pushFunc);
        rpc->bind(func_prefix + "_Pop", popFunc);
        break;
      }
#endif
    }
  }

  explicit concurrent_queue(CharStruct name_ = "TEST_CONCURRENT_QUEUE",
                            uint16_t port = HCL_CONF->RPC_PORT)
      : container(name_, port) {
    queue = nullptr;
    AutoTrace trace = AutoTrace("hcl::concurrent_queue");
    if (is_server) {
      queue = new queue_type(128);
      bind_functions();
    } else if (!is_server && server_on_node) {
    }
  }

  queue_type *data() { return queue; }

  bool LocalPush(ValueT &v) { return queue->push(v); }
  std::pair<bool, ValueT> LocalPop() {
    ValueT v;
    bool b = queue->pop(v);
    if (b)
      return std::pair<bool, ValueT>(true, v);
    else
      return std::pair<bool, ValueT>(false, ValueT());
  }

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalPush, (v), ValueT &v)
  THALLIUM_DEFINE1(LocalPop)
#endif

  bool Push(uint64_t &s, ValueT &v);
  std::pair<bool, ValueT> Pop(uint64_t &s);
};

#include "queue.cpp"

}  // namespace hcl

#endif
