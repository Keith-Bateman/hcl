#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>
#include <hcl/concurrent/skiplist/skiplist.h>
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

template <class T, class HashFcn, class Comp, class NodeAlloc, int MAX_HEIGHT>
uint64_t concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                             MAX_HEIGHT>::power_of_two(int n) {
  int c = 1;
  while (c < n) {
    c = 2 * c;
  }
  return c;
}
template <class T, class HashFcn, class Comp, class NodeAlloc, int MAX_HEIGHT>
uint64_t concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                             MAX_HEIGHT>::serverLocation(T &k) {
  uint64_t id = -1;
  uint64_t hashval = HashFcn()(k);
  uint64_t mask = UINT64_MAX;
  mask = mask << (64 - nbits);
  id = hashval & mask;
  id = id >> (64 - nbits);
  if (id >= nservers) id = nservers - 1;
  return id;
}
template <class T, class HashFcn, class Comp, class NodeAlloc, int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::isLocal(
    T &k) {
  if (is_server && serverLocation(k) == serverid)
    return true;
  else
    return false;
}
template <class T, class HashFcn, class Comp, class NodeAlloc, int MAX_HEIGHT>
void concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                         MAX_HEIGHT>::initialize_sets(uint32_t np,
                                                      uint32_t rank) {
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
template <class T, class HashFcn, class Comp, class NodeAlloc, int MAX_HEIGHT>
concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                    MAX_HEIGHT>::~concurrent_skiplist() {
  if (a != nullptr) delete a;
  if (s != nullptr) delete s;
}

template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::Insert(
    T &key) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace = AutoTrace("hcl::concurrent_skiplist::Insert(remote)", key);
  return RPC_CALL_WRAPPER("_Insert", key_int, bool, key);
}

template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::Find(
    T &key) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace = AutoTrace("hcl::concurrent_skiplist::Find(remote)", key);
  return RPC_CALL_WRAPPER("_Find", key_int, bool, key);
}

template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::Erase(
    T &key) {
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  AutoTrace trace = AutoTrace("hcl::concurrent_skiplist::Erase(remote)", key);
  return RPC_CALL_WRAPPER("_Erase", key_int, bool, key);
}
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
void concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                         MAX_HEIGHT>::construct_shared_memory() {}
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
void concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                         MAX_HEIGHT>::open_shared_memory() {}
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
void concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                         MAX_HEIGHT>::bind_functions() {
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
    case RPCLIB: {
      std::function<bool(T &)> insertFunc(
          std::bind(&concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                                         MAX_HEIGHT>::LocalInsert,
                    this, std::placeholders::_1));
      std::function<bool(T &)> findFunc(
          std::bind(&concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                                         MAX_HEIGHT>::LocalFind,
                    this, std::placeholders::_1));
      std::function<bool(T &)> eraseFunc(
          std::bind(&concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                                         MAX_HEIGHT>::LocalErase,
                    this, std::placeholders::_1));

      rpc->bind(func_prefix + "_Insert", insertFunc);
      rpc->bind(func_prefix + "_Find", findFunc);
      rpc->bind(func_prefix + "_Erase", eraseFunc);
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
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
concurrent_skiplist<T, HashFcn, Comp, NodeAlloc,
                    MAX_HEIGHT>::concurrent_skiplist(CharStruct name_,
                                                     uint16_t port)
    : Container(name_, port) {
  a = nullptr;
  s = nullptr;
  AutoTrace trace = AutoTrace("hcl::concurrent_skiplist");
  if (is_server) {
    bind_functions();
  } else if (!is_server && server_on_node) {
  }
}
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::LocalInsert(
    T &k) {
  auto ret = a->insert(k);
  return ret.second;
}
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::LocalFind(
    T &k) {
  return a->contains(k);
}
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::LocalErase(
    T &k) {
  return a->remove(k);
}

}  // namespace hcl
