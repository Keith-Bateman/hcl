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
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
/**
 * Include Headers
 */
#include <hcl/base/containers/concurrent_skiplist/skip_list.h>
#include <hcl/common/container.h>
#include <hcl/common/macros.h>
#include <hcl/communication/rpc_factory.h>
#include <hcl/communication/rpc_lib.h>

/** Boost Headers **/
#include <boost/algorithm/string.hpp>
/** Standard C++ Headers**/
#include <float.h>

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
class concurrent_skiplist : public Container {
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

  uint64_t power_of_two(int n);

 public:
  uint64_t serverLocation(T &k);

  bool isLocal(T &k);

  void initialize_sets(uint32_t np, uint32_t rank);
  ~concurrent_skiplist();

  void construct_shared_memory() override;
  void open_shared_memory() override;
  void bind_functions() override;

  concurrent_skiplist(CharStruct name_ = "TEST_CONCURRENT_SKIPLIST",
                        uint16_t port = 0);

  bool LocalInsert(T &k);
  bool LocalFind(T &k);
  bool LocalErase(T &k);

  bool Insert(T &k);
  bool Find(T &k);
  bool Erase(T &k);

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalInsert, (k), T &k)
  THALLIUM_DEFINE(LocalFind, (k), T &k)
  THALLIUM_DEFINE(LocalErase, (k), T &k)
#endif
};
}  // namespace hcl

#endif
