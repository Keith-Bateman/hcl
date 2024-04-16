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

#ifndef INCLUDE_HCL_SEQUENCER_GLOBAL_SEQUENCE_H_
#define INCLUDE_HCL_SEQUENCER_GLOBAL_SEQUENCE_H_

#include <hcl/common/container.h>
#include <hcl/hcl_internal.h>
#include <stdint-gcc.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <hcl/hcl_config.hpp>
#include <memory>
#include <string>
#include <utility>

namespace bip = boost::interprocess;

namespace hcl {
class global_sequence : public container {
 private:
  uint64_t *value;

 public:
  ~global_sequence() { this->container::~container(); }

  void construct_shared_memory() override {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    value = segment.construct<uint64_t>(name.c_str())(0);
  }

  void open_shared_memory() override {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    std::pair<uint64_t *, bip::managed_mapped_file::size_type> res;
    res = segment.find<uint64_t>(name.c_str());
    value = res.first;
  }

  void bind_functions() override {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()

    auto rpc = hcl::HCL::GetInstance(false)->GetRPC(port);
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {
        std::function<void(const tl::request &)> getNextSequence(
            std::bind(&hcl::global_sequence::ThalliumLocalGetNextSequence, this,
                      std::placeholders::_1));
        rpc->bind(func_prefix + "_GetNextSequence", getNextSequence);
        break;
      }
#endif
    }
  }

  global_sequence(CharStruct name_ = "TEST_GLOBAL_SEQUENCE",
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
    if (is_server) {
      construct_shared_memory();
      bind_functions();
    } else if (!is_server && server_on_node) {
      open_shared_memory();
    }
  }
  uint64_t *data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (server_on_node || is_server) {
      HCL_CPP_FUNCTION_UPDATE("access", "local");
      return value;
    } else {
      return nullptr;
    }
  }
  uint64_t GetNextSequence() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (is_local()) {
      HCL_CPP_FUNCTION_UPDATE("access", "local");
      return LocalGetNextSequence();
    } else {
      auto my_server_i = my_server_idx;
      HCL_CPP_FUNCTION_UPDATE("access", "remote");
      HCL_CPP_FUNCTION_UPDATE("access", my_server_i);
      return RPC_CALL_WRAPPER1("_GetNextSequence", my_server_i, uint64_t);
    }
  }
  uint64_t GetNextSequenceServer(uint16_t &server) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (is_local(server)) {
      HCL_CPP_FUNCTION_UPDATE("access", "local");
      return LocalGetNextSequence();
    } else {
      HCL_CPP_FUNCTION_UPDATE("access", "remote");
      HCL_CPP_FUNCTION_UPDATE("access", server);
      return RPC_CALL_WRAPPER1("_GetNextSequence", server, uint64_t);
    }
  }

  uint64_t LocalGetNextSequence() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    return ++*value;
  }

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE1(LocalGetNextSequence)
#endif
};

}  // namespace hcl

#endif  // INCLUDE_HCL_SEQUENCER_GLOBAL_SEQUENCE_H_
