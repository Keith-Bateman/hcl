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

#include <hcl/concurrent/unordered_map/unordered_map.h>
/** Include Headers**/
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>
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

namespace hcl {

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::isLocal(
    KeyT &k) {
  uint64_t hashval = HashFcn()(k);
  uint64_t pos = hashval % totalSize;
  if (is_server && pos >= min_range && pos < max_range)
    return true;
  else
    return false;
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
uint64_t concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                  EqualFcn>::serverLocation(KeyT &k) {
  uint64_t localSize = totalSize / num_servers;
  uint64_t rem = totalSize % num_servers;
  uint64_t hashval = HashFcn()(k);
  uint64_t v = hashval % totalSize;
  uint64_t offset = rem * (localSize + 1);
  uint64_t id = -1;
  if (v >= 0 && v < totalSize) {
    if (v < offset)
      id = v / (localSize + 1);
    else
      id = rem + ((v - offset) / localSize);
  }

  return id;
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
void concurrent_unordered_map<KeyT, ValueT, HashFcn,
                              EqualFcn>::initialize_tables(uint64_t n,
                                                           uint32_t np,
                                                           uint32_t rank,
                                                           KeyT maxKey) {
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

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
concurrent_unordered_map<KeyT, ValueT, HashFcn,
                         EqualFcn>::~concurrent_unordered_map() {
  if (my_table != nullptr) delete my_table;
  if (pl != nullptr) delete pl;
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
void concurrent_unordered_map<KeyT, ValueT, HashFcn,
                              EqualFcn>::construct_shared_memory() {}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
void concurrent_unordered_map<KeyT, ValueT, HashFcn,
                              EqualFcn>::open_shared_memory() {}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
void concurrent_unordered_map<KeyT, ValueT, HashFcn,
                              EqualFcn>::bind_functions() {
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
    case RPCLIB: {
      std::function<bool(KeyT &, ValueT &)> insertFunc(
          std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                              EqualFcn>::LocalInsert,
                    this, std::placeholders::_1, std::placeholders::_2));
      std::function<bool(KeyT &)> findFunc(std::bind(
          &concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalFind,
          this, std::placeholders::_1));
      std::function<bool(KeyT &)> eraseFunc(
          std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                              EqualFcn>::LocalErase,
                    this, std::placeholders::_1));
      std::function<ValueT(KeyT &)> getFunc(
          std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                              EqualFcn>::LocalGetValue,
                    this, std::placeholders::_1));
      std::function<bool(KeyT &, ValueT &)> updateFunc(
          std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
                                              EqualFcn>::LocalUpdate,
                    this, std::placeholders::_1, std::placeholders::_2));

      rpc->bind(func_prefix + "_Insert", insertFunc);
      rpc->bind(func_prefix + "_Find", findFunc);
      rpc->bind(func_prefix + "_Erase", eraseFunc);
      rpc->bind(func_prefix + "_Get", getFunc);
      rpc->bind(func_prefix + "_Update", updateFunc);
      break;
    }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP:
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
    case THALLIUM_ROCE:
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
      std::function<void(const tl::request &, KeyT &)> getFunc(
          std::bind(&concurrent_unordered_map<KeyT, ValueT, HashFcn,
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

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
concurrent_unordered_map<KeyT, ValueT, HashFcn,
                         EqualFcn>::concurrent_unordered_map(CharStruct name_,
                                                             uint16_t port)
    : Container(name_, port) {
  my_table = nullptr;
  pl = nullptr;
  AutoTrace trace = AutoTrace("hcl::map");
  if (is_server) {
    bind_functions();
  } else if (!is_server && server_on_node) {
  }
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
BlockMap<KeyT, ValueT, HashFcn, EqualFcn> *
concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::data() {
  return my_table;
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalInsert(
    KeyT &k, ValueT &v) {
  uint32_t r = my_table->insert(k, v);
  if (r != NOT_IN_TABLE)
    return true;
  else
    return false;
}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalFind(
    KeyT &k) {
  if (my_table->find(k) != NOT_IN_TABLE)
    return true;
  else
    return false;
}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalErase(
    KeyT &k) {
  return my_table->erase(k);
}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalUpdate(
    KeyT &k, ValueT &v) {
  return my_table->update(k, v);
}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalGet(
    KeyT &k, ValueT *v) {
  return my_table->get(k, v);
}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
ValueT concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::LocalGetValue(
    KeyT &k) {
  ValueT v;
  new (&v) ValueT();
  bool b = LocalGet(k, &v);
  return v;
}


template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
uint64_t
concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::allocated() {
  return my_table->allocated_nodes();
}
template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
uint64_t concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::removed() {
  return my_table->removed_nodes();
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Insert(
    KeyT &key, ValueT &data) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace =
      AutoTrace("hcl::concurrent_unordered_map::Insert(remote)", key, data);
  return RPC_CALL_WRAPPER("_Insert", key_int, bool, key, data);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Find(
    KeyT &key) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace =
      AutoTrace("hcl::concurrent_unordered_map::Find(remote)", key);
  return RPC_CALL_WRAPPER("_Find", key_int, bool, key);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Erase(
    KeyT &key) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace =
      AutoTrace("hcl::concurrent_unordered_map::Erase(remote)", key);
  return RPC_CALL_WRAPPER("_Erase", key_int, bool, key);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
ValueT concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Get(
    KeyT &key) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace =
      AutoTrace("hcl::concurrent_unordered_map::Get(remote)", key);
  return RPC_CALL_WRAPPER("_Get", key_int, ValueT, key);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Update(
    KeyT &key, ValueT &data) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace =
      AutoTrace("hcl::concurrent_unordered_map::Update(remote)", key, data);
  return RPC_CALL_WRAPPER("_Update", key_int, bool, key, data);
}

}  // namespace hcl