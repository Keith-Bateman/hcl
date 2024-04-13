#ifndef HCL_SKIPLIST_ACCESSOR_H
#define HCL_SKIPLIST_ACCESSOR_H
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
#include <hcl/common/logging.h>
template <typename T, typename Comp, typename NodeAlloc, int MAX_HEIGHT>
class ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT>::Accessor {
  typedef SkipListNode<T> NodeType;
  typedef ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT> SkipListType;

 private:
  SkipListType* sl_;
  std::shared_ptr<SkipListType> slHolder_;

 public:
  typedef T value_type;
  typedef T key_type;
  typedef T& reference;
  typedef T* pointer;
  typedef const T& const_reference;
  typedef const T* const_pointer;
  typedef size_t size_type;
  typedef Comp key_compare;
  typedef Comp value_compare;

  typedef typename SkipListType::iterator iterator;
  typedef typename SkipListType::const_iterator const_iterator;
  typedef typename SkipListType::Skipper Skipper;

  explicit Accessor(std::shared_ptr<ConcurrentSkipList> skip_list)
      : slHolder_(std::move(skip_list)) {
    HCL_LOG_TRACE("Accessor::Accessor(skip_list)");
    sl_ = slHolder_.get();
    assert(sl_ != nullptr);
    sl_->recycler_.addRef();
  }

  explicit Accessor(ConcurrentSkipList* skip_list) : sl_(skip_list) {
    HCL_LOG_TRACE("Accessor::Accessor(skip_list*)");
    assert(sl_ != nullptr);
    sl_->recycler_.addRef();
  }

  Accessor(const Accessor& accessor)
      : sl_(accessor.sl_), slHolder_(accessor.slHolder_) {
    HCL_LOG_TRACE("Accessor::Accessor(copy)");
    sl_->recycler_.addRef();
  }

  Accessor& operator=(const Accessor& accessor) {
    HCL_LOG_TRACE("Accessor::operator=");
    if (this != &accessor) {
      slHolder_ = accessor.slHolder_;
      sl_->recycler_.releaseRef();
      sl_ = accessor.sl_;
      sl_->recycler_.addRef();
    }
    return *this;
  }

  ~Accessor() { sl_->recycler_.releaseRef(); }

  bool empty() const {
    HCL_LOG_TRACE("Accessor::empty");
    return sl_->size() == 0;
  }
  size_t size() const {
    HCL_LOG_TRACE("Accessor::size");
    return sl_->size();
  }
  size_type max_size() const {
    HCL_LOG_TRACE("Accessor::max_size");
    return std::numeric_limits<size_type>::max();
  }

  iterator find(const key_type& value) {
    HCL_LOG_TRACE("Accessor::find");
    return iterator(sl_->find(value));
  }
  const_iterator find(const key_type& value) const {
    HCL_LOG_TRACE("Accessor::find");
    return iterator(sl_->find(value));
  }
  size_type count(const key_type& data) const {
    HCL_LOG_TRACE("Accessor::count");
    return contains(data);
  }

  iterator begin() const {
    HCL_LOG_TRACE("Accessor::begin");
    NodeType* head = sl_->head_.load(std::memory_order_acquire);
    return iterator(head->next());
  }
  iterator end() const {
    HCL_LOG_TRACE("Accessor::end");
    return iterator(nullptr);
  }
  const_iterator cbegin() const {
    HCL_LOG_TRACE("Accessor::cbegin");
    return begin();
  }
  const_iterator cend() const {
    HCL_LOG_TRACE("Accessor::cend");
    return end();
  }

  template <typename U, typename = typename std::enable_if<
                            std::is_convertible<U, T>::value>::type>
  std::pair<iterator, bool> insert(U&& data) {
    HCL_LOG_TRACE("Accessor::insert");
    auto ret = sl_->addOrGetData(std::forward<U>(data));
    return std::make_pair(iterator(ret.first), ret.second);
  }
  size_t erase(const key_type& data) {
    HCL_LOG_TRACE("Accessor::erase");
    return remove(data);
  }

  iterator lower_bound(const key_type& data) const {
    HCL_LOG_TRACE("Accessor::lower_bound");
    return iterator(sl_->lower_bound(data));
  }

  size_t height() const {
    HCL_LOG_TRACE("Accessor::height");
    return sl_->height();
  }

  const key_type* first() const {
    HCL_LOG_TRACE("Accessor::first");
    return sl_->first();
  }
  const key_type* last() const {
    HCL_LOG_TRACE("Accessor::last");
    return sl_->last();
  }

  bool pop_back() {
    HCL_LOG_TRACE("Accessor::pop_back");
    auto last = sl_->last();
    return last ? sl_->remove(*last) : false;
  }

  std::pair<key_type*, bool> addOrGetData(const key_type& data) {
    HCL_LOG_TRACE("Accessor::addOrGetData");
    auto ret = sl_->addOrGetData(data);
    return std::make_pair(&ret.first->data(), ret.second);
  }

  SkipListType* skiplist() const {
    HCL_LOG_TRACE("Accessor::skiplist");
    return sl_;
  }

  bool contains(const key_type& data) const {
    HCL_LOG_TRACE("Accessor::contains");
    return sl_->find(data);
  }
  bool add(const key_type& data) {
    HCL_LOG_TRACE("Accessor::add");
    return sl_->addOrGetData(data).second;
  }
  bool remove(const key_type& data) {
    HCL_LOG_TRACE("Accessor::remove");
    return sl_->remove(data);
  }
};

template <class D, class V, class Tag>
class IteratorFacade {
 public:
  using value_type = V;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = ssize_t;
  using iterator_category = Tag;

  friend bool operator==(D const& lhs, D const& rhs) {
    HCL_LOG_TRACE("IteratorFacade::operator==");
    return equal(lhs, rhs);
  }

  friend bool operator!=(D const& lhs, D const& rhs) {
    HCL_LOG_TRACE("IteratorFacade::operator!=");
    return !(lhs == rhs);
  }

  V& operator*() const {
    HCL_LOG_TRACE("IteratorFacade::operator*");
    return asDerivedConst().dereference();
  }

  V* operator->() const {
    HCL_LOG_TRACE("IteratorFacade::operator->");
    return std::addressof(operator*());
  }

  D& operator++() {
    HCL_LOG_TRACE("IteratorFacade::operator++");
    asDerived().increment();
    return asDerived();
  }

  D operator++(int) {
    HCL_LOG_TRACE("IteratorFacade::operator++=int");
    auto ret = asDerived();
    asDerived().increment();
    return ret;
  }

  D& operator--() {
    HCL_LOG_TRACE("IteratorFacade::operator--");
    asDerived().decrement();
    return asDerived();
  }

  D operator--(int) {
    HCL_LOG_TRACE("IteratorFacade::operator--=int");
    auto ret = asDerived();
    asDerived().decrement();
    return ret;
  }

 private:
  D& asDerived() {
    HCL_LOG_TRACE("IteratorFacade::asDerived");
    return static_cast<D&>(*this);
  }

  D const& asDerivedConst() const {
    HCL_LOG_TRACE("IteratorFacade::asDerivedConst");
    return static_cast<D const&>(*this);
  }

  static bool equal(D const& lhs, D const& rhs) {
    HCL_LOG_TRACE("IteratorFacade::equal");
    return lhs.equal(rhs);
  }
};

template <typename ValT, typename NodeT>
class csl_iterator : public IteratorFacade<csl_iterator<ValT, NodeT>, ValT,
                                           std::forward_iterator_tag> {
 public:
  typedef ValT value_type;
  typedef value_type& reference;
  typedef value_type* pointer;
  typedef ptrdiff_t difference_type;

  explicit csl_iterator(NodeT* node = nullptr) : node_(node) {}

  template <typename OtherVal, typename OtherNode>
  csl_iterator(
      const csl_iterator<OtherVal, OtherNode>& other,
      typename std::enable_if<
          std::is_convertible<OtherVal*, ValT*>::value>::type* = nullptr)
      : node_(other.node_) {}

  size_t nodeSize() const {
    HCL_LOG_TRACE("csl_iterator::nodeSize");
    return node_ == nullptr ? 0
                            : node_->height() * sizeof(NodeT*) + sizeof(*this);
  }

  bool good() const {
    HCL_LOG_TRACE("csl_iterator::good");
    return node_ != nullptr;
  }

 private:
  template <class, class>
  friend class csl_iterator;
  friend class IteratorFacade<csl_iterator, ValT, std::forward_iterator_tag>;

  void increment() {
    HCL_LOG_TRACE("csl_iterator::increment");
    node_ = node_->next();
  }
  bool equal(const csl_iterator& other) const {
    HCL_LOG_TRACE("csl_iterator::equal");
    return node_ == other.node_;
  }
  value_type& dereference() const {
    HCL_LOG_TRACE("csl_iterator::dereference");
    return node_->data();
  }

  NodeT* node_;
};

template <typename T, typename Comp, typename NodeAlloc, int MAX_HEIGHT>
class ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT>::Skipper {
  typedef SkipListNode<T> NodeType;
  typedef ConcurrentSkipList<T, Comp, NodeAlloc, MAX_HEIGHT> SkipListType;
  typedef typename SkipListType::Accessor Accessor;

 public:
  typedef T value_type;
  typedef T& reference;
  typedef T* pointer;
  typedef ptrdiff_t difference_type;

  Skipper(std::shared_ptr<SkipListType> skipList)
      : accessor_(std::move(skipList)) {
    HCL_LOG_TRACE("Skipper::Skipper");
    init();
  }

  Skipper(const Accessor& accessor) : accessor_(accessor) {
    HCL_LOG_TRACE("Skipper::Skipper copy");
    init();
  }

  void init() {
    HCL_LOG_TRACE("Skipper::init");
    NodeType* head_node = head();
    headHeight_ = head_node->height();
    for (int i = 0; i < headHeight_; ++i) {
      preds_[i] = head_node;
      succs_[i] = head_node->skip(i);
    }
    int max_layer = maxLayer();
    for (int i = 0; i < max_layer; ++i) {
      hints_[i] = uint8_t(i + 1);
    }
    hints_[max_layer] = max_layer;
  }

  Skipper& operator++() {
    HCL_LOG_TRACE("Skipper::operator++");
    preds_[0] = succs_[0];
    succs_[0] = preds_[0]->skip(0);
    int height = curHeight();
    for (int i = 1; i < height && preds_[0] == succs_[i]; ++i) {
      preds_[i] = succs_[i];
      succs_[i] = preds_[i]->skip(i);
    }
    return *this;
  }

  Accessor& accessor() {
    HCL_LOG_TRACE("Skipper::accessor");
    return accessor_;
  }
  const Accessor& accessor() const {
    HCL_LOG_TRACE("Skipper::accessor const");
    return accessor_;
  }

  bool good() const {
    HCL_LOG_TRACE("Skipper::good");
    return succs_[0] != nullptr;
  }

  int maxLayer() const {
    HCL_LOG_TRACE("Skipper::maxLayer");
    return headHeight_ - 1;
  }

  int curHeight() const {
    HCL_LOG_TRACE("Skipper::curHeight");
    return succs_[0] ? std::min(headHeight_, succs_[0]->height()) : 0;
  }

  const value_type& data() const {
    HCL_LOG_TRACE("Skipper::value_type");
    assert(succs_[0] != nullptr);
    return succs_[0]->data();
  }

  value_type& operator*() const {
    HCL_LOG_TRACE("Skipper::operator*");
    assert(succs_[0] != nullptr);
    return succs_[0]->data();
  }

  value_type* operator->() {
    HCL_LOG_TRACE("Skipper::operator->");
    assert(succs_[0] != nullptr);
    return &succs_[0]->data();
  }

  bool to(const value_type& data) {
    HCL_LOG_TRACE("Skipper::to");
    int layer = curHeight() - 1;
    if (layer < 0) {
      return false;
    }

    int lyr = hints_[layer];
    int max_layer = maxLayer();
    while (SkipListType::greater(data, succs_[lyr]) && lyr < max_layer) {
      ++lyr;
    }
    hints_[layer] = lyr;

    int foundLayer = SkipListType::findInsertionPoint(preds_[lyr], lyr, data,
                                                      preds_, succs_);
    if (foundLayer < 0) {
      return false;
    }

    assert(succs_[0] != nullptr);
    return !succs_[0]->markedForRemoval();
  }

 private:
  NodeType* head() const {
    HCL_LOG_TRACE("Skipper::head");
    return accessor_.skiplist()->head_.load(std::memory_order_acquire);
  }

  Accessor accessor_;
  int headHeight_;
  NodeType *succs_[MAX_HEIGHT], *preds_[MAX_HEIGHT];
  uint8_t hints_[MAX_HEIGHT];
};

#endif
