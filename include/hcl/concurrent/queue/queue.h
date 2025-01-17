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

#include <hcl/hcl_config.hpp>
/**
 * Include Headers
 */

#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>
#include <hcl/communication/rpc_lib.h>
#include <hcl/hcl_internal.h>

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

  void construct_shared_memory() override {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  }
  void open_shared_memory() override {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  }
  void bind_functions() override {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()

    auto rpc = hcl::HCL::GetInstance(false)->GetRPC(port);
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM:
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

  explicit concurrent_queue(
      CharStruct name_ = "TEST_CONCURRENT_QUEUE",
      uint16_t port = HCL_CONF->RPC_PORT,
      uint16_t _num_servers = HCL_CONF->NUM_SERVERS,
      uint16_t _my_server_idx = HCL_CONF->MY_SERVER,
      really_long _memory_allocated = HCL_CONF->MEMORY_ALLOCATED,
      bool _is_server = HCL_CONF->IS_SERVER,
      bool _is_server_on_node = HCL_CONF->SERVER_ON_NODE,
      CharStruct _backed_file_dir = HCL_CONF->BACKED_FILE_DIR)
      : container(name_, port, _num_servers, _my_server_idx, _memory_allocated,
                  _is_server, _is_server_on_node, _backed_file_dir) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    queue = nullptr;
    if (is_server) {
      queue = new queue_type(128);
      bind_functions();
    } else if (!is_server && server_on_node) {
    }
  }

  queue_type *data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return queue;
  }

  bool LocalPush(ValueT &v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return queue->push(v);
  }
  std::pair<bool, ValueT> LocalPop() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
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
