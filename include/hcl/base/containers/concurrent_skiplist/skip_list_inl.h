#ifndef HCL_SKIPLIST_INL_H
#define HCL_SKIPLIST_INL_H
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>

#include <algorithm>
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/random.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <climits>
#include <cmath>
#include <memory>
#include <mutex>
#include <random>
#include <type_traits>
#include <vector>

template <typename ValT, typename NodeT>
class csl_iterator;

template <typename T>
class SkipListNode {
  enum : uint16_t {
    IS_HEAD_NODE = 1,
    MARKED_FOR_REMOVAL = (1 << 1),
    FULLY_LINKED = (1 << 2),
  };

 public:
  typedef T value_type;

  SkipListNode(const SkipListNode&) = delete;
  SkipListNode& operator=(const SkipListNode&) = delete;

  template <typename NodeAlloc, typename U,
            typename =
                typename std::enable_if<std::is_convertible<U, T>::value>::type>
  static SkipListNode* create(NodeAlloc& alloc, int height, U&& data,
                              bool isHead = false) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(height >= 1 && height < 64);

    size_t size =
        sizeof(SkipListNode) + height * sizeof(std::atomic<SkipListNode*>);
    auto storage = std::allocator_traits<NodeAlloc>::allocate(alloc, size);
    return new (storage)
        SkipListNode(uint8_t(height), std::forward<U>(data), isHead);
  }

  template <typename NodeAlloc>
  static void destroy(NodeAlloc& alloc, SkipListNode* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    size_t size = sizeof(SkipListNode) +
                  node->height_ * sizeof(std::atomic<SkipListNode*>);
    node->~SkipListNode();
    std::allocator_traits<NodeAlloc>::deallocate(
        alloc, typename std::allocator_traits<NodeAlloc>::pointer(node), size);
  }

  SkipListNode* copyHead(SkipListNode* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(node != nullptr && height_ > node->height_);
    setFlags(node->getFlags());
    for (uint8_t i = 0; i < node->height_; ++i) {
      setSkip(i, node->skip(i));
    }
    return this;
  }

  inline SkipListNode* skip(int layer) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(layer < height_);
    return skip_[layer].load(std::memory_order_acquire);
  }

  SkipListNode* next() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    SkipListNode* node;
    for (node = skip(0); (node != nullptr && node->markedForRemoval());
         node = node->skip(0)) {
    }
    return node;
  }

  void setSkip(uint8_t h, SkipListNode* next) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(h < height_);
    skip_[h].store(next, std::memory_order_release);
  }

  value_type& data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return data_;
  }
  const value_type& data() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return data_;
  }
  int maxLayer() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return height_ - 1;
  }
  int height() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return height_;
  }

  bool acquireGuard() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    spinLock_.lock();
    return true;
  }

  bool releaseGuard() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    spinLock_.unlock();
    return true;
  }

  uint16_t getFlags() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return flags_.load(std::memory_order_acquire);
  }
  void setFlags(uint16_t flags) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    flags_.store(flags, std::memory_order_release);
  }
  bool fullyLinked() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return getFlags() & FULLY_LINKED;
  }
  bool markedForRemoval() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return getFlags() & MARKED_FOR_REMOVAL;
  }
  bool isHeadNode() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return getFlags() & IS_HEAD_NODE;
  }

  void setIsHeadNode() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    setFlags(uint16_t(getFlags() | IS_HEAD_NODE));
  }
  void setFullyLinked() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    setFlags(uint16_t(getFlags() | FULLY_LINKED));
  }
  void setMarkedForRemoval() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    setFlags(uint16_t(getFlags() | MARKED_FOR_REMOVAL));
  }

  void storeData(value_type& data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    new (&data_) value_type(std::forward<value_type>(data));
  }
  void storeData(const value_type& data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    new (&data_) value_type(data);
  }

 private:
  template <typename U>
  SkipListNode(uint8_t height, U&& data, bool isHead)
      : height_(height), data_(std::forward<U>(data)) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    setFlags(0);
    if (isHead) {
      setIsHeadNode();
    }
    for (uint8_t i = 0; i < height_; ++i) {
      new (&skip_[i]) std::atomic<SkipListNode*>(nullptr);
    }
  }

  ~SkipListNode() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    for (uint8_t i = 0; i < height_; ++i) {
      skip_[i].~atomic();
    }
  }

  std::atomic<uint16_t> flags_;
  const uint8_t height_;
  boost::mutex spinLock_;

  value_type data_;

  std::atomic<SkipListNode*> skip_[1];
};

class SkipListRandomHeight {
  enum { kMaxHeight = 64 };

 public:
  std::default_random_engine rd;
  std::uniform_real_distribution<double> dist{0, 1};
  std::function<double()> r;

  static SkipListRandomHeight* instance() {
    static SkipListRandomHeight instance_;
    return &instance_;
  }

  int getHeight(int maxHeight) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(maxHeight <= kMaxHeight);
    double p = r();
    for (int i = 0; i < maxHeight; ++i) {
      if (p < lookupTable_[i]) {
        return i + 1;
      }
    }
    return maxHeight;
  }

  size_t getSizeLimit(int height) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(height < kMaxHeight);
    return sizeLimitTable_[height];
  }

  SkipListRandomHeight() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    int id = random() % 12;
    rd.seed(id);
    r = bind(dist, rd);
    initLookupTable();
  }

 private:
  void initLookupTable() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    static const double kProbInv = exp(1);
    static const double kProb = 1.0 / kProbInv;
    static const size_t kMaxSizeLimit = std::numeric_limits<size_t>::max();

    double sizeLimit = 1;
    double p = lookupTable_[0] = (1 - kProb);
    sizeLimitTable_[0] = 1;
    for (int i = 1; i < kMaxHeight - 1; ++i) {
      p *= kProb;
      sizeLimit *= kProbInv;
      lookupTable_[i] = lookupTable_[i - 1] + p;
      sizeLimitTable_[i] = sizeLimit > kMaxSizeLimit
                               ? kMaxSizeLimit
                               : static_cast<size_t>(sizeLimit);
    }
    lookupTable_[kMaxHeight - 1] = 1;
    sizeLimitTable_[kMaxHeight - 1] = kMaxSizeLimit;
  }

  double randomProb() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return r();
  }

  double lookupTable_[kMaxHeight];
  size_t sizeLimitTable_[kMaxHeight];
};

template <typename NodeType, typename NodeAlloc, typename = void>
class NodeRecycler;

template <typename NodeType, typename NodeAlloc>
class NodeRecycler<NodeType,
                   NodeAlloc>  //,typename std::enable_if<!NodeType::template
                               // DestroyIsNoOp<NodeAlloc>::value>::type>
{
 public:
  explicit NodeRecycler(const NodeAlloc& alloc)
      : refs_(0), dirty_(false), alloc_(alloc) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    chunk_size = 100;
    node_queues.resize(64);
    for (int i = 0; i < node_queues.size(); i++)
      node_queues[i] = new boost::lockfree::queue<NodeType*>(128);
  }

  explicit NodeRecycler() : refs_(0), dirty_(false) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    chunk_size = 100;
    node_queues.resize(64);
    for (long unsigned int i = 0; i < node_queues.size(); i++)
      node_queues[i] = new boost::lockfree::queue<NodeType*>(128);
  }

  ~NodeRecycler() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(refs() == 0);
    if (nodes_) {
      for (auto& node : *nodes_) {
        NodeType::destroy(alloc_, node);
      }
    }
    for (long unsigned int i = 0; i < node_queues.size(); i++)
      delete node_queues[i];
  }

  void push(int h, NodeType* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    assert(h >= 0 && h < 64);
    node_queues[h]->push(node);
    bool p = false;
    bool n = true;
    assert(refs() > 0);
    dirty_.compare_exchange_strong(p, n, std::memory_order_relaxed);
  }
  NodeType* pop(int h, bool ishead) {
    NodeType* n = nullptr;
    assert(h >= 0 && h < 64);
    typedef typename NodeType::value_type v;
    while (!node_queues[h]->pop(n)) {
      for (int i = 0; i < chunk_size; i++) {
        NodeType* nn = NodeType::create(alloc_, h, v());
        node_queues[h]->push(nn);
      }
      if (node_queues[h]->pop(n)) break;
    }

    assert(refs() >= 0);
    bool p = false;
    bool p_n = true;
    dirty_.compare_exchange_strong(p, p_n, std::memory_order_relaxed);
    n->setFlags(0);
    if (ishead) n->setIsHeadNode();
    return n;
  }

  void add(NodeType* node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    boost::unique_lock<boost::mutex> g(lock_);
    if (nodes_.get() == nullptr) {
      nodes_ = std::make_unique<std::vector<NodeType*>>(1, node);
    } else {
      nodes_->push_back(node);
    }
    assert(refs() > 0);
    dirty_.store(true, std::memory_order_relaxed);
  }

  int addRef() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return refs_.fetch_add(1, std::memory_order_acq_rel);
  }

  int releaseRef() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (!dirty_.load(std::memory_order_relaxed) || refs() > 1) {
      return refs_.fetch_add(-1, std::memory_order_acq_rel);
    }

    std::unique_ptr<std::vector<NodeType*>> newNodes;
    int ret;
    {
      HCL_LOG_TRACE();
      HCL_CPP_FUNCTION()
      boost::unique_lock<boost::mutex> g(lock_);
      ret = refs_.fetch_add(-1, std::memory_order_acq_rel);
      if (ret == 1) {
        for (int i = 0; i < 64; i++) {
          while (!node_queues[i]->empty()) {
            NodeType* n;
            if (node_queues[i]->pop(n)) NodeType::destroy(alloc_, n);
          }
        }
        newNodes.swap(nodes_);
        dirty_.store(false, std::memory_order_relaxed);
      }
    }
    if (newNodes) {
      for (auto& node : *newNodes) {
        NodeType::destroy(alloc_, node);
      }
    }
    return ret;
  }

  NodeAlloc& alloc() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return alloc_;
  }

 private:
  int refs() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return refs_.load(std::memory_order_relaxed);
  }

  std::vector<boost::lockfree::queue<NodeType*>*> node_queues;
  std::unique_ptr<std::vector<NodeType*>> nodes_;
  std::atomic<int32_t> refs_;
  std::atomic<bool> dirty_;
  boost::mutex lock_;
  NodeAlloc alloc_;
  int chunk_size;
};

#endif
