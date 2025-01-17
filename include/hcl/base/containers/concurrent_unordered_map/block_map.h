#ifndef HCL_BLOCK_MAP_H
#define HCL_BLOCK_MAP_H

#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <hcl/hcl_config.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include "memory_allocation.h"

#define NOT_IN_TABLE UINT64_MAX
#define EXISTS 1
#define INSERTED 0

/*This file contains the shared memory implementation of the concurrent
 * unordered map, which is distributed using HCL RPC wrappers. It is a static
 * unordered map of fixed size - which removes the complexities of rehashing.
 * The nodes of the unordered map are allocated and reused using a memory
 * allocator*/

namespace hcl {

template <class KeyT, class ValueT, class HashFcn = std::hash<KeyT>,
          class EqualFcn = std::equal_to<KeyT>>
struct f_node {
  uint64_t num_nodes;
  std::mutex mutex_t;
  struct node<KeyT, ValueT, HashFcn, EqualFcn> *head;
};

template <class KeyT, class ValueT, class HashFcn = std::hash<KeyT>,
          class EqualFcn = std::equal_to<KeyT>>
class BlockMap {
 public:
  typedef struct node<KeyT, ValueT, HashFcn, EqualFcn> node_type;
  typedef struct f_node<KeyT, ValueT, HashFcn, EqualFcn> fnode_type;

 private:
  fnode_type *table;
  uint64_t maxSize;
  std::atomic<uint64_t> allocated;
  std::atomic<uint64_t> removed;
  memory_pool<KeyT, ValueT, HashFcn, EqualFcn> *pl;
  KeyT emptyKey;

  uint64_t KeyToIndex(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t hashval = HashFcn()(k);
    return hashval % maxSize;
  }

 public:
  BlockMap(uint64_t n, memory_pool<KeyT, ValueT, HashFcn, EqualFcn> *m,
           KeyT maxKey)
      : maxSize(n), pl(m), emptyKey(maxKey) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(maxSize > 0);
    table = (fnode_type *)std::malloc(maxSize * sizeof(fnode_type));
    assert(table != nullptr);
    for (size_t i = 0; i < maxSize; i++) {
      table[i].num_nodes = 0;
      table[i].head = pl->memory_pool_pop();
      new (&(table[i].head->key)) KeyT(emptyKey);
      table[i].head->next = nullptr;
    }
    allocated.store(0);
    removed.store(0);
    assert(maxSize < UINT64_MAX);
  }

  ~BlockMap() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    std::free(table);
  }

  uint32_t insert(KeyT &k, ValueT &v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t pos = KeyToIndex(k);

    table[pos].mutex_t.lock();

    node_type *p = table[pos].head;
    node_type *n = table[pos].head->next;

    bool found = false;
    while (n != nullptr) {
      if (EqualFcn()(n->key, k)) found = true;
      if (HashFcn()(n->key) > HashFcn()(k)) {
        break;
      }
      p = n;
      n = n->next;
    }

    uint32_t ret = (found) ? EXISTS : 0;
    if (!found) {
      allocated.fetch_add(1);
      node_type *new_node = pl->memory_pool_pop();
      new (&(new_node->key)) KeyT(k);
      new (&(new_node->value)) ValueT(v);
      new_node->next = n;
      p->next = new_node;
      table[pos].num_nodes++;
      found = true;
      ret = INSERTED;
    }

    table[pos].mutex_t.unlock();
    return ret;
  }

  uint64_t find(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t pos = KeyToIndex(k);

    table[pos].mutex_t.lock();

    node_type *n = table[pos].head->next;
    bool found = false;
    while (n != nullptr) {
      if (EqualFcn()(n->key, k)) {
        found = true;
      }
      if (HashFcn()(n->key) > HashFcn()(k)) break;
      n = n->next;
    }

    table[pos].mutex_t.unlock();

    return (found ? pos : NOT_IN_TABLE);
  }

  bool update(KeyT &k, ValueT &v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t pos = KeyToIndex(k);

    table[pos].mutex_t.lock();

    node_type *n = table[pos].head->next;

    bool found = false;
    while (n != nullptr) {
      if (EqualFcn()(n->key, k)) {
        found = true;
        n->value = v;
      }
      if (HashFcn()(n->key) > HashFcn()(k)) break;
      n = n->next;
    }

    table[pos].mutex_t.unlock();
    return found;
  }

  bool get(KeyT &k, ValueT *v) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    bool found = false;

    uint64_t pos = KeyToIndex(k);

    table[pos].mutex_t.lock();

    node_type *n = table[pos].head;

    while (n != nullptr) {
      if (EqualFcn()(n->key, k)) {
        found = true;
        *v = n->value;
      }
      if (HashFcn()(n->key) > HashFcn()(k)) break;
      n = n->next;
    }

    table[pos].mutex_t.unlock();

    return found;
  }
  // clang-format off
  template <typename... Args>
  bool update_field(KeyT &k, void (*fn)(ValueT *, Args &&...args),
                    Args &&...args_) {
    // clang-format on
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    bool found = false;
    uint64_t pos = KeyToIndex(k);

    table[pos].mutex_t.lock();

    node_type *n = table[pos].head->next;

    while (n != nullptr) {
      if (EqualFcn()(n->key, k)) {
        found = true;
        fn(&(n->value), std::forward<Args>(args_)...);
      }
      if (HashFcn()(n->key) > HashFcn()(k)) break;
      n = n->next;
    }

    table[pos].mutex_t.unlock();

    return found;
  }

  bool erase(KeyT &k) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t pos = KeyToIndex(k);

    table[pos].mutex_t.lock();

    node_type *p = table[pos].head;
    node_type *n = table[pos].head->next;

    bool found = false;

    while (n != nullptr) {
      if (EqualFcn()(n->key, k)) break;

      if (HashFcn()(n->key) > HashFcn()(k)) break;
      p = n;
      n = n->next;
    }

    if (n != nullptr)
      if (EqualFcn()(n->key, k)) {
        found = true;
        p->next = n->next;
        pl->memory_pool_push(n);
        table[pos].num_nodes--;
        removed.fetch_add(1);
      }

    table[pos].mutex_t.unlock();
    return found;
  }

  uint64_t allocated_nodes() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return allocated.load();
  }

  uint64_t removed_nodes() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return removed.load();
  }

  uint64_t count_block_entries() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    uint64_t num_entries = 0;
    for (size_t i = 0; i < maxSize; i++) {
      num_entries += table[i].num_nodes;
    }
    return num_entries;
  }
};

}  // namespace hcl

#endif
