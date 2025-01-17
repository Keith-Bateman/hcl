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

#ifndef INCLUDE_HCL_CONCURRENT_SKIPLIST_H_
#define INCLUDE_HCL_CONCURRENT_SKIPLIST_H_

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
#include <hcl/base/containers/concurrent_skiplist/skip_list.h>
#include <hcl/common/container.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/*This file contains the class that implements a distributed concurrent set. The
 * total size of the set is not fixed. Each server has a set. The total key
 * space is partitioned across servers using the upper log(n) bits where 'n' is
 * the number of servers. A client program should locate the server for its key
 * and make RPC calls to it for performing set operations. The underlying set is
 * implemented using a concurrent randomized skiplist. The skiplist can be
 * accessed concurrently using multiple threads.*/

namespace hcl {

template <class T, class HashFcn = std::hash<T>, class Comp = std::less<T>,
          class NodeAlloc = std::allocator<char>, int MAX_HEIGHT = 24>
class concurrent_skiplist : public container {
  typedef ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT> SkipListType;
  typedef typename ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT>::Accessor
      SkipListAccessor;

 private:
  uint64_t totalSize;
  uint32_t nservers;
  uint32_t serverid;
  uint64_t nbits;
  SkipListType *s;
  SkipListAccessor *a;

  uint64_t power_of_two(int n) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    int c = 1;
    while (c < n) {
      c = 2 * c;
    }
    return c;
  }

 public:
  uint64_t serverLocation(T &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t id = -1;
    uint64_t hashval = HashFcn()(k);
    uint64_t mask = UINT64_MAX;
    mask = mask << (64 - nbits);
    id = hashval & mask;
    id = id >> (64 - nbits);
    if (id >= nservers) id = nservers - 1;
    return id;
  }

  bool isLocal(T &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (is_server && serverLocation(k) == serverid)
      return true;
    else
      return false;
  }

  void initialize_sets(uint32_t np, uint32_t rank) {
    nservers = np;
    serverid = rank;
    uint64_t nservers_2 = power_of_two(nservers);
    nbits = log2(nservers_2);

    s = nullptr;
    a = nullptr;
    if (is_server) {
      s = new SkipListType(2);
      a = new SkipListAccessor(s);
    }
  }
  ~concurrent_skiplist() {
    if (a != nullptr) delete a;
    if (s != nullptr) delete s;
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

        std::function<void(const tl::request &, T &)> insertFunc(
            std::bind(&concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                                           MAX_HEIGHT>::ThalliumLocalInsert,
                      this, std::placeholders::_1, std::placeholders::_2));
        std::function<void(const tl::request &, T &)> findFunc(
            std::bind(&concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                                           MAX_HEIGHT>::ThalliumLocalFind,
                      this, std::placeholders::_1, std::placeholders::_2));
        std::function<void(const tl::request &, T &)> eraseFunc(
            std::bind(&concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                                           MAX_HEIGHT>::ThalliumLocalErase,
                      this, std::placeholders::_1, std::placeholders::_2));

        rpc->bind(func_prefix + "_Insert", insertFunc);
        rpc->bind(func_prefix + "_Find", findFunc);
        rpc->bind(func_prefix + "_Erase", eraseFunc);
        break;
      }
#endif
    }
  }

  explicit concurrent_skiplist(
      CharStruct name_ = "TEST_CONCURRENT_SKIPLIST",
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
    a = nullptr;
    s = nullptr;
    if (is_server) {
      bind_functions();
    } else if (!is_server && server_on_node) {
    }
  }

  bool LocalInsert(T &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    auto ret = a->insert(k);
    return ret.second;
  }
  bool LocalFind(T &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return a->contains(k);
  }
  bool LocalErase(T &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return a->remove(k);
  }

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalInsert, (k), T &k)
  THALLIUM_DEFINE(LocalFind, (k), T &k)
  THALLIUM_DEFINE(LocalErase, (k), T &k)
#endif

  bool Insert(T &k);
  bool Find(T &k);
  bool Erase(T &k);
};
#include "skiplist.cpp"
}  // namespace hcl

#endif
