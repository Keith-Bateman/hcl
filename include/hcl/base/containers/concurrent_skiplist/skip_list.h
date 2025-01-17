#ifndef HCL_SKIPLIST_H
#define HCL_SKIPLIST_H

#include <hcl/hcl_config.hpp>

#include "skip_list_inl.h"

/*This file contains the class that implements a concurrent randomized skiplist.
 * It is based on folly's skiplist implementation. But we diverged a bit from
 * folly. We have used a custom memory management module based on lockfree
 * queues to recycle used nodes. We have also used locks from boost, instead of
 * microlocks in folly. Folly's implementation is based on the skiplist
 * algorithm with optimistic locks which uses linked and mark bits to indicate
 * linked/deleted nodes. This algorithm avoids locks during search/find
 * operations, but acquires locks for insertion/deletion*/

template <typename T, typename Comp = std::less<T>,
          typename NodeAlloc = std::allocator<char>, int MAX_HEIGHT = 24>
class ConcurrentSkipList {
  typedef ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT> SkipListType;

 public:
  typedef SkipListNode<T> NodeType;
  typedef T value_type;
  typedef T key_type;

  typedef csl_iterator<value_type, NodeType> iterator;
  typedef csl_iterator<const value_type, NodeType> const_iterator;

  class Accessor;
  class Skipper;

  explicit ConcurrentSkipList(int height, const NodeAlloc& alloc)
      : recycler_(alloc),
        head_(
            recycler_.pop(height, true))  // NodeType::create(recycler_.alloc(),
                                          // height, value_type(), true))
  {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  }

  explicit ConcurrentSkipList(int height)
      : recycler_(),
        head_(
            recycler_.pop(height, true))  // NodeType::create(recycler_.alloc(),
                                          // height, value_type(), true))
  {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  }

  static Accessor create(int height, const NodeAlloc& alloc) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return Accessor(createInstance(height, alloc));
  }

  static Accessor create(int height = 1) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return Accessor(createInstance(height));
  }

  static std::shared_ptr<SkipListType> createInstance(int height,
                                                      const NodeAlloc& alloc) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return std::make_shared<ConcurrentSkipList>(height, alloc);
  }

  static std::shared_ptr<SkipListType> createInstance(int height = 1) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return std::make_shared<ConcurrentSkipList>(height);
  }

  size_t size() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return size_.load(std::memory_order_relaxed);
  }
  bool empty() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return size() == 0;
  }

  ~ConcurrentSkipList() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    /*if (NodeType::template DestroyIsNoOp<NodeAlloc>::value)
    {
      return;
    }*/
    for (NodeType* current = head_.load(std::memory_order_relaxed); current;) {
      NodeType* tmp = current->skip(0);
      NodeType::destroy(recycler_.alloc(), current);
      current = tmp;
    }
  }

 private:
  static bool greater(const value_type& data, const NodeType* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return node && Comp()(node->data(), data);
  }

  static bool less(const value_type& data, const NodeType* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return (node == nullptr) || Comp()(data, node->data());
  }

  static int findInsertionPoint(NodeType* cur, int cur_layer,
                                const value_type& data, NodeType* preds[],
                                NodeType* succs[]) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    int foundLayer = -1;
    NodeType* pred = cur;
    NodeType* foundNode = nullptr;
    for (int layer = cur_layer; layer >= 0; --layer) {
      NodeType* node = pred->skip(layer);
      while (greater(data, node)) {
        pred = node;
        node = node->skip(layer);
      }
      if (foundLayer == -1 && !less(data, node)) {
        foundLayer = layer;
        foundNode = node;
      }
      preds[layer] = pred;

      succs[layer] = foundNode ? foundNode : node;
    }
    return foundLayer;
  }

  int height() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return head_.load(std::memory_order_acquire)->height();
  }

  int maxLayer() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return height() - 1;
  }

  size_t incrementSize(int delta) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return size_.fetch_add(delta, std::memory_order_relaxed) + delta;
  }

  NodeType* find(const value_type& data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    auto ret = findNode(data);
    if (ret.second && !ret.first->markedForRemoval()) {
      return ret.first;
    }
    return nullptr;
  }

  bool lockNodesForChange(int nodeHeight, bool guards[MAX_HEIGHT],
                          NodeType* preds[MAX_HEIGHT],
                          NodeType* succs[MAX_HEIGHT], bool adding = true) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    NodeType *pred, *succ, *prevPred = nullptr;
    bool valid = true;
    for (int layer = 0; valid && layer < nodeHeight; ++layer) {
      pred = preds[layer];
      assert(pred != nullptr);
      succ = succs[layer];
      if (pred != prevPred) {
        guards[layer] = pred->acquireGuard();
        prevPred = pred;
      }
      valid = !pred->markedForRemoval() && pred->skip(layer) == succ;

      if (adding) {
        valid = valid && (succ == nullptr || !succ->markedForRemoval());
      }
    }

    return valid;
  }

  template <typename U>
  std::pair<NodeType*, size_t> addOrGetData(U&& data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    NodeType *preds[MAX_HEIGHT], *succs[MAX_HEIGHT];
    NodeType* newNode;
    size_t newSize;
    while (true) {
      int max_layer = 0;
      int layer = findInsertionPointGetMaxLayer(data, preds, succs, &max_layer);

      if (layer >= 0) {
        NodeType* nodeFound = succs[layer];
        assert(nodeFound != nullptr);
        if (nodeFound->markedForRemoval()) {
          continue;
        }
        while (!nodeFound->fullyLinked()) {
        }
        return std::make_pair(nodeFound, 0);
      }

      int nodeHeight =
          SkipListRandomHeight::instance()->getHeight(max_layer + 1);

      bool guards[MAX_HEIGHT];
      for (int i = 0; i < MAX_HEIGHT; i++) guards[i] = false;
      if (!lockNodesForChange(nodeHeight, guards, preds, succs)) {
        for (int i = 0; i < MAX_HEIGHT; i++)
          if (guards[i]) preds[i]->releaseGuard();
        continue;
      }

      newNode = recycler_.pop(nodeHeight,
                              false);  // NodeType::create(recycler_.alloc(),
                                       // nodeHeight, std::forward<U>(data));
      newNode->storeData(std::forward<U>(data));
      for (int k = 0; k < nodeHeight; ++k) {
        newNode->setSkip(k, succs[k]);
        preds[k]->setSkip(k, newNode);
      }

      newNode->setFullyLinked();
      newSize = incrementSize(1);
      for (int i = 0; i < MAX_HEIGHT; i++)
        if (guards[i]) preds[i]->releaseGuard();
      break;
    }

    int hgt = height();
    size_t sizeLimit = SkipListRandomHeight::instance()->getSizeLimit(hgt);

    if (hgt < MAX_HEIGHT && newSize > sizeLimit) {
      growHeight(hgt + 1);
    }
    assert(newSize > 0);
    return std::make_pair(newNode, newSize);
  }

  bool remove(const value_type& data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    NodeType* nodeToDelete = nullptr;
    bool isMarked = false;
    int nodeHeight = 0;
    NodeType *preds[MAX_HEIGHT], *succs[MAX_HEIGHT];

    while (true) {
      int max_layer = 0;
      int layer = findInsertionPointGetMaxLayer(data, preds, succs, &max_layer);
      if (!isMarked && (layer < 0 || !okToDelete(succs[layer], layer))) {
        return false;
      }

      if (!isMarked) {
        nodeToDelete = succs[layer];
        nodeHeight = nodeToDelete->height();
        nodeToDelete->acquireGuard();
        if (nodeToDelete->markedForRemoval()) {
          nodeToDelete->releaseGuard();
          return false;
        }
        nodeToDelete->setMarkedForRemoval();
        nodeToDelete->releaseGuard();
        isMarked = true;
      }

      bool guards[MAX_HEIGHT];
      for (int i = 0; i < MAX_HEIGHT; i++) guards[i] = false;
      if (!lockNodesForChange(nodeHeight, guards, preds, succs, false)) {
        for (int i = 0; i < MAX_HEIGHT; i++)
          if (guards[i]) preds[i]->releaseGuard();
        continue;
      }

      for (int k = nodeHeight - 1; k >= 0; --k) {
        preds[k]->setSkip(k, nodeToDelete->skip(k));
      }

      incrementSize(-1);
      for (int i = 0; i < MAX_HEIGHT; i++)
        if (guards[i]) preds[i]->releaseGuard();
      break;
    }
    recycler_.push(nodeToDelete->height(), nodeToDelete);
    // recycle(nodeToDelete);
    return true;
  }

  const value_type* first() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    auto node = head_.load(std::memory_order_acquire)->skip(0);
    return node ? &node->data() : nullptr;
  }

  const value_type* last() const {
    NodeType* pred = head_.load(std::memory_order_acquire);
    NodeType* node = nullptr;
    for (int layer = maxLayer(); layer >= 0; --layer) {
      HCL_LOG_TRACE();
      HCL_CPP_FUNCTION()
      do {
        node = pred->skip(layer);
        if (node) {
          pred = node;
        }
      } while (node != nullptr);
    }
    return pred == head_.load(std::memory_order_relaxed) ? nullptr
                                                         : &pred->data();
  }

  static bool okToDelete(NodeType* candidate, int layer) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(candidate != nullptr);
    return candidate->fullyLinked() && candidate->maxLayer() == layer &&
           !candidate->markedForRemoval();
  }

  int findInsertionPointGetMaxLayer(const value_type& data, NodeType* preds[],
                                    NodeType* succs[], int* max_layer) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    *max_layer = maxLayer();
    return findInsertionPoint(head_.load(std::memory_order_acquire), *max_layer,
                              data, preds, succs);
  }

  std::pair<NodeType*, int> findNode(const value_type& data) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return findNodeDownRight(data);
  }

  std::pair<NodeType*, int> findNodeDownRight(const value_type& data) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    NodeType* pred = head_.load(std::memory_order_acquire);
    int ht = pred->height();
    NodeType* node = nullptr;

    bool found = false;
    while (!found) {
      for (; ht > 0 && less(data, node = pred->skip(ht - 1)); --ht) {
      }
      if (ht == 0) {
        return std::make_pair(node, 0);
      }
      --ht;

      while (greater(data, node)) {
        pred = node;
        node = node->skip(ht);
      }
      found = !less(data, node);
    }
    return std::make_pair(node, found);
  }

  std::pair<NodeType*, int> findNodeRightDown(const value_type& data) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    NodeType* pred = head_.load(std::memory_order_acquire);
    NodeType* node = nullptr;
    auto top = maxLayer();
    int found = 0;
    for (int layer = top; !found && layer >= 0; --layer) {
      node = pred->skip(layer);
      while (greater(data, node)) {
        pred = node;
        node = node->skip(layer);
      }
      found = !less(data, node);
    }
    return std::make_pair(node, found);
  }

  NodeType* lower_bound(const value_type& data) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    auto node = findNode(data).first;
    while (node != nullptr && node->markedForRemoval()) {
      node = node->skip(0);
    }
    return node;
  }

  void growHeight(int height) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    NodeType* oldHead = head_.load(std::memory_order_acquire);
    if (oldHead->height() >= height) {
      return;
    }

    NodeType* newHead = recycler_.pop(height, true);
    // NodeType::create(recycler_.alloc(), height, value_type(), true);

    {
      oldHead->acquireGuard();
      newHead->copyHead(oldHead);
      NodeType* expected = oldHead;
      if (!head_.compare_exchange_strong(expected, newHead,
                                         std::memory_order_release)) {
        NodeType::destroy(recycler_.alloc(), newHead);
        oldHead->releaseGuard();
        return;
      }
      oldHead->setMarkedForRemoval();
      oldHead->releaseGuard();
    }
    recycler_.push(oldHead->height(), oldHead);
    // recycle(oldHead);
  }

  void recycle(NodeType* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    recycler_.add(node);
  }

  NodeRecycler<NodeType, NodeAlloc> recycler_;
  std::atomic<NodeType*> head_;
  std::atomic<size_t> size_{0};
};

#include "skip_list_accessor.h"

#endif
