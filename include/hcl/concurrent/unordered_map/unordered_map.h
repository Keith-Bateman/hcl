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

#ifndef INCLUDE_HCL_CONCURRENT_UNORDERED_MAP_H_
#define INCLUDE_HCL_CONCURRENT_UNORDERED_MAP_H_

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
/** Standard C++ Headers**/
#include <float.h>
#include <hcl/base/containers/concurrent_unordered_map/block_map.h>
#include <hcl/common/container.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/*This file contains the class that implements a distributed concurrent
 * unordered map of fixed size. The total size of the map can be configured by
 * the user. The map is partitioned and distributed across servers. A client
 * program can use the HCL container to locate the server containing a key and
 * make an RPC call for remote map operations. The unordered map on each server
 * is concurrent, which means it can be accessed by multiple threads
 * simultaneously.*/

namespace hcl {

template <class KeyT, class ValueT, class HashFcn = std::hash<KeyT>,
          class EqualFcn = std::equal_to<KeyT>>
class concurrent_unordered_map : public container {
 public:
  typedef BlockMap<KeyT, ValueT, HashFcn, EqualFcn> map_type;
  typedef memory_pool<KeyT, ValueT, HashFcn, EqualFcn> pool_type;

 private:
  uint64_t totalSize;
  uint64_t maxSize;
  uint64_t min_range;
  uint64_t max_range;
  uint32_t nservers;
  uint32_t serverid;
  KeyT emptyKey;
  pool_type *pl;
  map_type *my_table;

 public:
  bool isLocal(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t hashval = HashFcn()(k);
    uint64_t pos = hashval % totalSize;
    if (is_server && pos >= min_range && pos < max_range)
      return true;
    else
      return false;
  }

  uint64_t serverLocation(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t localSize = totalSize / num_servers;
    uint64_t rem = totalSize % num_servers;
    uint64_t hashval = HashFcn()(k);
    uint64_t v = hashval % totalSize;
    uint64_t offset = rem * (localSize + 1);
    uint64_t id = -1;
    if (v < totalSize) {
      if (v < offset)
        id = v / (localSize + 1);
      else
        id = rem + ((v - offset) / localSize);
    }

    return id;
  }

  void initialize_tables(uint64_t n, uint32_t np, uint32_t rank, KeyT maxKey) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    totalSize = n;
    nservers = np;
    serverid = rank;
    emptyKey = maxKey;
    my_table = nullptr;
    pl = nullptr;
    assert(totalSize > 0 && totalSize < UINT64_MAX);
    uint64_t localSize = totalSize / nservers;
    uint64_t rem = totalSize % nservers;
    if (serverid < rem)
      maxSize = localSize + 1;
    else
      maxSize = localSize;
    assert(maxSize > 0 && maxSize < UINT64_MAX);
    min_range = 0;

    if (serverid < rem)
      min_range = serverid * (localSize + 1);
    else
      min_range = rem * (localSize + 1) + (serverid - rem) * localSize;

    max_range = min_range + maxSize;

    if (is_server) {
      pl = new pool_type(100);
      my_table = new map_type(maxSize, pl, emptyKey);
    }
  }

  ~concurrent_unordered_map() {
    if (my_table != nullptr) delete my_table;
    if (pl != nullptr) delete pl;
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
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {

        std::function<void(const tl::request &, KeyT &, ValueT &)> insertFunc(
            std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                                EqualFcn>::ThalliumLocalInsert,
                      this, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));
        std::function<void(const tl::request &, KeyT &)> findFunc(
            std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                                EqualFcn>::ThalliumLocalFind,
                      this, std::placeholders::_1, std::placeholders::_2));
        std::function<void(const tl::request &, KeyT &)> eraseFunc(
            std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                                EqualFcn>::ThalliumLocalErase,
                      this, std::placeholders::_1, std::placeholders::_2));
        std::function<void(const tl::request &, KeyT &)> getFunc(std::bind(
            &concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                      EqualFcn>::ThalliumLocalGetValue,
            this, std::placeholders::_1, std::placeholders::_2));
        std::function<void(const tl::request &, KeyT &, ValueT &)> updateFunc(
            std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                                EqualFcn>::ThalliumLocalUpdate,
                      this, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));

        rpc->bind(func_prefix + "_Insert", insertFunc);
        rpc->bind(func_prefix + "_Find", findFunc);
        rpc->bind(func_prefix + "_Erase", eraseFunc);
        rpc->bind(func_prefix + "_Get", getFunc);
        rpc->bind(func_prefix + "_Update", updateFunc);
        break;
      }
#endif
    }
  }

  explicit concurrent_unordered_map(
      CharStruct name_ = "TEST_UNORDERED_MAP_CONCURRENT",
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
    my_table = nullptr;
    pl = nullptr;
    if (is_server) {
      bind_functions();
    } else if (!is_server && server_on_node) {
    }
  }

  map_type *data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return my_table;
  }

  bool LocalInsert(KeyT &k, ValueT &v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    my_table->insert(k, v);
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return true;
  }
  bool LocalFind(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    if (my_table->find(k) != NOT_IN_TABLE)
      return true;
    else
      return false;
  }
  bool LocalErase(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return my_table->erase(k);
  }
  bool LocalUpdate(KeyT &k, ValueT &v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return my_table->update(k, v);
  }
  bool LocalGet(KeyT &k, ValueT *v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return my_table->get(k, v);
  }
  ValueT LocalGetValue(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    ValueT v;
    new (&v) ValueT();
    LocalGet(k, &v);
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return v;
  }
  // clang-format off
  template <typename... Args>
  bool LocalUpdateField(KeyT &k, void (*f)(ValueT *, Args &&...args),
                        Args &&...args_) {
    // clang-format on
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return my_table->update_field(k, f, std::forward<Args>(args_)...);
  }

  uint64_t allocated() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return my_table->allocated_nodes();
  }

  uint64_t removed() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return my_table->removed_nodes();
  }

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalInsert, (k, v), KeyT &k, ValueT &v)
  THALLIUM_DEFINE(LocalFind, (k), KeyT &k)
  THALLIUM_DEFINE(LocalErase, (k), KeyT &k)
  THALLIUM_DEFINE(LocalGetValue, (k), KeyT &k)
  THALLIUM_DEFINE(LocalUpdate, (k, v), KeyT &k, ValueT &v)
#endif

  bool Insert(KeyT &k, ValueT &v);
  bool Find(KeyT &k);
  bool Erase(KeyT &k);
  ValueT Get(KeyT &k);
  bool Update(KeyT &k, ValueT &v);
};

#include "unordered_map.cpp"

}  // namespace hcl

#endif
