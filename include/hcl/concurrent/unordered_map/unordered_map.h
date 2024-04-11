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
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
/**
 * Include Headers
 */

#include <hcl/common/macros.h>
#include <hcl/communication/rpc_factory.h>
#include <hcl/communication/rpc_lib.h>

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
class concurrent_unordered_map : public Container {
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
  bool isLocal(KeyT &k);

  uint64_t serverLocation(KeyT &k);

  void initialize_tables(uint64_t n, uint32_t np, uint32_t rank, KeyT maxKey);

  ~concurrent_unordered_map();

  void construct_shared_memory() override;
  void open_shared_memory() override;
  void bind_functions() override;

  concurrent_unordered_map(CharStruct name_ = "TEST_UNORDERED_MAP_CONCURRENT",
                           uint16_t port = 0);

  map_type *data();

  bool LocalInsert(KeyT &k, ValueT &v);
  bool LocalFind(KeyT &k);
  bool LocalErase(KeyT &k);
  bool LocalUpdate(KeyT &k, ValueT &v);
  bool LocalGet(KeyT &k, ValueT *v);
  ValueT LocalGetValue(KeyT &k);

  template <typename... Args>
  bool LocalUpdateField(KeyT &k, void (*f)(ValueT *, Args &&...args),
                        Args &&...args_) {
    return my_table->update_field(k, f, std::forward<Args>(args_)...);
  }

  uint64_t allocated();

  uint64_t removed();

  bool Insert(KeyT &k, ValueT &v);
  bool Find(KeyT &k);
  bool Erase(KeyT &k);
  ValueT Get(KeyT &k);
  bool Update(KeyT &k, ValueT &v);
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalInsert, (k, v), KeyT &k, ValueT &v)
  THALLIUM_DEFINE(LocalFind, (k), KeyT &k)
  THALLIUM_DEFINE(LocalErase, (k), KeyT &k)
  THALLIUM_DEFINE(LocalGetValue, (k), KeyT &k)
  THALLIUM_DEFINE(LocalUpdate, (k, v), KeyT &k, ValueT &v)
#endif
};

}  // namespace hcl

#endif
