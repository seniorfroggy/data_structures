#include <iostream>
#include <vector>
#include <cmath>
#include <tuple>

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:

  struct BaseNode {
    BaseNode *prev = nullptr;
    BaseNode *next = nullptr;
    BaseNode() : prev(nullptr), next(nullptr) {}
    BaseNode(BaseNode *first, BaseNode *second) : prev(first), next(second) {}
  };

  struct Node : public BaseNode {
    T value;

    Node() : BaseNode(nullptr, nullptr) {};
    Node(BaseNode *left, BaseNode *right, const T &value) : BaseNode(left, right), value(value) {}
    Node(BaseNode *left, BaseNode *right, T &&value) : BaseNode(left, right), value(std::move(value)) {}
    Node(const T &value) : value(value) {}
    Node(T &&value) : value(std::move(value)) {}
    template<typename... Args>
    Node(BaseNode *left, BaseNode *right, Args &&... args): BaseNode(left, right), value(std::forward<Args>(args)...) {}
    template<typename... Args>
    Node(Args &&... args): BaseNode(nullptr, nullptr), value(std::forward<Args>(args)...) {}
  };

 public:
  template<typename Key,
      typename Value,
      typename Hash,
      typename Equal,
      typename Allocator>
  friend
  class UnorderedMap;
  typedef typename std::allocator_traits<Alloc>::template rebind_alloc<Node> NodeAllocator;

 private:
  BaseNode fakeNode_;
  size_t size_;
  NodeAllocator node_allocator_;
  Alloc alloc_;

  void clean_up(size_t sz) {
    BaseNode *curr = fakeNode_.next;
    for (size_t i = 0; i < sz; ++i) {
      BaseNode *temp;
      temp = curr->next;
      AllocTraits::destroy(node_allocator_, reinterpret_cast<Node *>(curr));
      AllocTraits::deallocate(node_allocator_, reinterpret_cast<Node *>(curr), 1);
      curr = temp;
    }
    size_ = 0;
  }

 public:

  void clear() {
    clean_up(size());
  }
  template<typename V>
  struct base_iterator {
   public:
    using difference_type = ptrdiff_t;
    using value_type = V;
    using pointer = V *;
    using reference = value_type &;
    using iterator_category = std::bidirectional_iterator_tag;

   private:
    BaseNode *curr_;
    void swap_iter(base_iterator &it) { std::swap(curr_, it.curr_); }

   public:

    ~base_iterator() = default;

    base_iterator() { curr_ = nullptr; }

    base_iterator(const BaseNode *node) : curr_(const_cast<BaseNode *>(node)) {}

    base_iterator(BaseNode *node) : curr_(node) {}

    base_iterator(const base_iterator &it) : curr_(it.curr_) {}

    value_type &operator*() {
      return reinterpret_cast<Node *>(curr_)->value;
    }

    value_type *operator->() {
      return &(reinterpret_cast<Node *>(curr_)->value);
    }

    base_iterator &operator=(base_iterator it) {
      swap_iter(it);
      return *this;
    }
    base_iterator &operator++() {
      *this = base_iterator(curr_->next);
      return *this;
    }

    base_iterator operator++(int) {
      auto temp = *this;
      ++*this;
      return temp;
    }

    base_iterator operator--() {
      return *this = base_iterator(curr_->prev);
    }

    base_iterator operator--(int) {
      auto temp = *this;
      --*this;
      return temp;
    }

    BaseNode *get_node_ptr() const {
      return curr_;
    }

    bool operator==(const base_iterator& b) const {
      return curr_ == b.curr_;
    }
    bool operator!=(const base_iterator& b) const {
      return curr_ != b.curr_;
    }
    operator base_iterator<const V>() const {
      return base_iterator<const V>(curr_);
    }
  };

  using iterator = base_iterator<T>;
  using const_iterator = base_iterator<const T>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using AllocTraits = typename std::allocator_traits<NodeAllocator>;


  List() : fakeNode_(&fakeNode_, &fakeNode_), size_(0) {}

  List(const Alloc &allocator) : fakeNode_(&fakeNode_, &fakeNode_),
                                 size_(0),
                                 node_allocator_(allocator),
                                 alloc_(allocator) {}


  List &operator=(const List &other) {
    List temp = other;
    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      node_allocator_ = other.node_allocator_;
      alloc_ = other.alloc_;
    }
    std::swap(fakeNode_, temp.fakeNode_);
    std::swap(size_, temp.size_);
    return *this;
  }
  void copyList(List& other) {
    fakeNode_.next = other.fakeNode_.next;
    fakeNode_.prev = other.fakeNode_.prev;
    fakeNode_.next->prev = &fakeNode_;
    fakeNode_.prev->next = &fakeNode_;
    other.fakeNode_.prev = nullptr;
    other.fakeNode_.next = nullptr;
    other.size_ = 0;
  }

  List(List &&other) noexcept
      : size_(other.size_),
        node_allocator_(std::move(other.node_allocator_)) {
    copyList(other);
  }

  List &operator=(List &&other) noexcept {
    if (this != &other) {
      clear();
      size_ = other.size_;
      copyList(other);
    }
    return *this;
  }

  iterator begin() {
    return iterator(fakeNode_.next);
  }

  iterator end() {
    return iterator(&fakeNode_);
  }

  const_iterator begin() const {
    return cbegin();
  }

  const_iterator end() const {
    return cend();
  }

  const_iterator cbegin() {
    return const_iterator(fakeNode_.next);
  }

  const_iterator cend() {
    return const_iterator(&fakeNode_);
  }

  const_iterator cbegin() const {
    return const_iterator(fakeNode_.next);
  }

  const_iterator cend() const {
    return const_iterator(&fakeNode_);
  }

  reverse_iterator rbegin() {
    return std::reverse_iterator<iterator>(end());
  }

  reverse_iterator rend() {
    return std::reverse_iterator<iterator>(begin());
  }

  const_reverse_iterator rbegin() const {
    return crbegin();
  }

  const_reverse_iterator rend() const {
    return crend();
  }

  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  const_reverse_iterator crbegin() {
    return std::reverse_iterator<const_iterator>(cend());
  }

  const_reverse_iterator crend() {
    return std::reverse_iterator<const_iterator>(cbegin());
  }

  void insert(const_iterator it, const T &value) {
    BaseNode *right = it.get_node_ptr();
    BaseNode *left = right->prev;
    BaseNode *to_be_inserted;
    try {
      to_be_inserted = AllocTraits::allocate(node_allocator_, 1);
      AllocTraits::construct(node_allocator_, reinterpret_cast<Node *>(to_be_inserted), left, right, value);
    } catch (...) {
      throw;
    }
    right->prev = to_be_inserted;
    left->next = to_be_inserted;
    ++size_;
  }

  template<typename... Args>
  void emplace(const_iterator it, Args &&... args) {
    BaseNode *right = it.get_node_ptr();
    BaseNode *left = right->prev;
    Node *to_be_inserted;
    try {
      to_be_inserted = AllocTraits::allocate(node_allocator_, 1);
      std::allocator_traits<Alloc>::construct(alloc_,
                                              reinterpret_cast<T *>(&to_be_inserted->value),
                                              std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    to_be_inserted->next = right;
    to_be_inserted->prev = left;
    right->prev = to_be_inserted;
    left->next = to_be_inserted;
    ++size_;
  }

  void insert_before(BaseNode *node, BaseNode *curr) {
    BaseNode *prev_prev = curr->prev;
    BaseNode *prev = node->prev;
    prev_prev->next = node;
    if (prev != nullptr and node->next != nullptr) {
      prev->next = node->next;
      node->next->prev = prev;
    } else {
      ++size_;
    }
    node->next = curr;
    node->prev = curr->prev;
    curr->prev = node;
  }

  void insert(const_iterator it, T &&value) {
    emplace(it, std::forward<T>(value));
  }

  void erase(const_iterator it) {
    BaseNode *to_be_erased = it.get_node_ptr();
    BaseNode *left = to_be_erased->prev;
    BaseNode *right = to_be_erased->next;
    left->next = right;
    right->prev = left;
    AllocTraits::destroy(node_allocator_, reinterpret_cast<Node *>(to_be_erased));
    AllocTraits::deallocate(node_allocator_, reinterpret_cast<Node *>(to_be_erased), 1);
    --size_;
  }

  NodeAllocator get_allocator() {
    return node_allocator_;
  }
  size_t size() const {
    return size_;
  }
  void push_back(const T &value) {
    insert(end(), value);
  }
  void push_back(T &&value) {
    emplace(cend(), std::forward<T>(value));
  }
  void push_front(const T &value) {
    insert(begin(), value);
  }
  void push_front(T &&value) {
    emplace(begin(), std::forward<T>(value));
  }
  void pop_front() {
    erase(begin());
  }
  void pop_back() {
    erase(--end());
  }

  ~List() {
    clean_up(size_);
  }

 List(size_t size, Alloc allocator = Alloc()): fakeNode_(&fakeNode_, &fakeNode_),
                                                      node_allocator_(allocator), alloc_(allocator) {
    BaseNode *previous = &fakeNode_;
    Node *curr;
    size_ = 0;
    for (size_t i = 0; i < size; ++i) {
      try {
        curr = AllocTraits::allocate(node_allocator_, 1);
      } catch (...) {
        clean_up(size_);
        throw;
      }
      try {
        AllocTraits::construct(node_allocator_, curr);
      } catch (...) {
        AllocTraits::deallocate(node_allocator_, curr, 1);
        clean_up(size_);
        throw;
      }
      curr->prev = previous;
      previous->next = curr;
      previous = curr;
      ++size_;
    }
    fakeNode_.prev = curr;
    curr->next = &fakeNode_;
  }

  List(size_t size, const T &value, Alloc allocator = Alloc()) : fakeNode_(&fakeNode_, &fakeNode_),
                                                                       size_(size), node_allocator_(allocator),
                                                                       alloc_(allocator) {
    BaseNode *previous = &fakeNode_;
    Node *curr;
    for (size_t i = 0; i < size_; ++i) {
      try {
        curr = AllocTraits::allocate(node_allocator_, 1);
      } catch (...) {
        clean_up(i);
        throw;
      }
      try {
        AllocTraits::construct(node_allocator_, curr, value);
      } catch (...) {
        AllocTraits::deallocate(node_allocator_, curr, 1);
        clean_up(i);
        throw;
      }
      curr->prev = previous;
      previous->next = curr;
      previous = curr;
    }
    fakeNode_.prev = curr;
    curr->next = &fakeNode_;
  }

  List(const List &other)
      : List(AllocTraits::select_on_container_copy_construction(other.node_allocator_)) {
    auto it = other.begin();
    size_ = 0;
    for (size_t i = 0; i < other.size(); ++i) {
      try {
        push_back(*it);
        ++it;
      } catch (...) {
        clean_up(size_);
        throw;
      }
    }
  }
};






// ================================||======||*||\*****||*||======||*||======||**||******||*****||======*=========================================================================================
// ================================||******||*||*\****||*||******||*||******||**||********||***||*******================================================================================
// ================================||******||*||**\***||*||******||*||======||**||**********||*||======*================================================================================
// ================================||******||*||***\**||*||******||*||******\***||**********||*||======*==============================================================================
// ================================||******||*||****\*||*||******||*||*******\**||*********||**||*******==============================================================================
// ================================||======||*||*****\||*||======||*||********\*||*******||****||======*============================================================================================








template <typename Key,
          typename Value,
          typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
 public:
  using NodeType = std::pair<const Key, Value>;
 private:
  using NodeTypeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<NodeType>;
  using BucketsType = typename List<NodeType, NodeTypeAlloc>::iterator;
  using BucketsAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<BucketsType>;

  Alloc alloc_;
  List<NodeType, NodeTypeAlloc> list_;
  std::vector<BucketsType, BucketsAlloc> buckets_;
  Hash hashFunc_;
  Equal equalFunc_;
  float max_load_factor_ = 0.8;
  static const size_t default_size_ = 32;

  void check_for_rehash() {
    if ((list_.size() / buckets_.size()) > max_load_factor_) {
      rehash(2 * buckets_.size());
    }
  }

  void rehash(size_t n) {
    std::vector<BucketsType, BucketsAlloc> new_buckets(n, list_.end(), alloc_);

    iterator curr = list_.begin();
    while (curr != list_.end()) {
      size_t hash = hashFunc_(curr->first) % n;
      if (new_buckets[hash] != list_.end()) {
        list_.insert_before(curr++.get_node_ptr(), new_buckets[hash].get_node_ptr());
      } else {
        new_buckets[hash] = curr++;
      }
    }
    buckets_ = std::move(new_buckets);
  }

 public:
  using iterator = typename List<NodeType, NodeTypeAlloc>::iterator;;
  using const_iterator = typename List<NodeType, NodeTypeAlloc>::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using AllocTraits = typename std::allocator_traits<Alloc>;

  iterator begin() {
    return list_.begin();
  }

  iterator end() {
    return list_.end();
  }

  const_iterator begin() const {
    return list_.cbegin();
  }

  const_iterator end() const {
    return list_.cend();
  }

  const_iterator cbegin() {
    return list_.cbegin();
  }

  const_iterator cend() {
    return list_.cend();
  }

  const_iterator cbegin() const {
    return cbegin();
  }

  const_iterator cend() const {
    return cend();
  }

  reverse_iterator rbegin() {
    return std::reverse_iterator<iterator>(end());
  }

  reverse_iterator rend() {
    return std::reverse_iterator<iterator>(begin());
  }

  const_reverse_iterator rbegin() const {
    return crbegin();
  }

  const_reverse_iterator rend() const {
    return crend();
  }

  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  const_reverse_iterator crbegin() {
    return std::reverse_iterator<const_iterator>(cend());
  }

  const_reverse_iterator crend() {
    return std::reverse_iterator<const_iterator>(cbegin());
  }







  // ======================================================================================================================================================================






  UnorderedMap()
      : alloc_(Alloc()),
        list_(alloc_),
        buckets_(default_size_, list_.end(), alloc_),
        hashFunc_(Hash()),
        equalFunc_(Equal()) {}

  UnorderedMap(const Alloc& allocator): UnorderedMap() {
    alloc_ = allocator;
  }

  ~UnorderedMap() {
    clear();
  }

  UnorderedMap(UnorderedMap&& other) noexcept
      : alloc_(std::move(other.alloc_)),
        list_(std::move(other.list_)),
        buckets_(std::move(other.buckets_)),
        hashFunc_(std::move(other.hashFunc_)),
        equalFunc_(std::move(other.equalFunc_)),
        max_load_factor_(other.max_load_factor_)
  {
    rehash(buckets_.size());
    other.clear();
  }

  void swapMap(UnorderedMap& other) {
    list_ = std::move(other.list_);
    buckets_ = std::move(other.buckets_);
    hashFunc_ = std::move(other.hashFunc_);
    equalFunc_ = std::move(other.equalFunc_);
    max_load_factor_ = other.max_load_factor_;
  }

  UnorderedMap& operator=(UnorderedMap&& other) noexcept {
    if (this != &other) {
      auto temp = std::move(other);
      std::swap(alloc_, temp.alloc_);
      swapMap(temp);
      rehash(buckets_.size());
    }
    return *this;
  }

  UnorderedMap(const UnorderedMap& other):
        list_(other.list_),
        hashFunc_(other.hashFunc_),
        equalFunc_(other.equalFunc_),
        max_load_factor_(other.max_load_factor_)
        {
    alloc_ = AllocTraits::select_on_container_copy_construction(other.alloc_);
    rehash(other.buckets_.size());
  }

  UnorderedMap& operator=(const UnorderedMap& other) {
    UnorderedMap temp = other;
    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }
    swapMap(temp);
    return *this;
  }






  // ==========================================================================================================================================================================








  void clear() {
    list_.clear();
    buckets_.clear();
    buckets_.resize(default_size_);
  }

  void reserve(size_t n) {
    if (static_cast<double>(n) <= (static_cast<double>(buckets_.size()) * max_load_factor_ )) {
      return;
    }
    size_t new_bucket_count = std::ceil(static_cast<double>(n) / max_load_factor_);
    rehash(new_bucket_count);
  }

  size_t size() {
    return list_.size();
  }

  iterator find(const Key& key) {
    size_t hash = hashFunc_(key) % buckets_.size();
    auto it = buckets_[hash];
    while (it != list_.end() and (hashFunc_(it->first) % buckets_.size()) == hash) {
      if (equalFunc_(it->first, key)) {
        break;
      }
      ++it;
    }
    if (it != list_.end() and equalFunc_(it->first, key)) {
      return it;
    }
    return list_.end();
  }

  const_iterator find(const Key& key) const {
    if (list_.size() == 0) {
      return list_.end();
    }
    size_t hash = hashFunc_(key) % buckets_.size();
    auto it = buckets_[hash];
    while (it != list_.end() and hashFunc_(it->first) == hash) {
      if (equalFunc_(it->first, key)) {
        break;
      }
      ++it;
    }
    if (it != list_.end() and equalFunc_(it->first, key)) {
      return it;
    }
    return list_.end();
  }

  Value& operator[](const Key& key) {
    auto it = find(key);
    size_t hash = hashFunc_(key) % buckets_.size();
    if (it != list_.end()) {
      return it->second;
    }
    if (buckets_[hash] != list_.end()) {
      list_.insert(buckets_[hash], NodeType(key, Value()));
      --buckets_[hash];
    } else {
      list_.push_back(NodeType(std::move(key), Value()));
    }
    check_for_rehash();
    return buckets_[hash]->second;
  }

  Value& operator[](Key&& key) {
    auto it = find(key);
    size_t hash = hashFunc_(key) % buckets_.size();
    if (it != list_.end()) {
      return it->second;
    }
    if (buckets_[hash] != list_.end()) {
      list_.insert(buckets_[hash], NodeType(std::move(key), Value()));
    } else {
      list_.push_back(NodeType(std::move(key), Value()));
    }
    --buckets_[hash];
    check_for_rehash();
    return buckets_[hash]->second;
  }

  Value& at(const Key& key) {
    auto it = find(key);
    if (it != list_.end()) {
      return it->second;
    }
    throw(std::out_of_range("out of range"));
  }

  Value& at(const Key& key) const {
    auto it = find(key);
    if (it != list_.end()) {
      return it->second;
    }
    throw(std::out_of_range("out of range"));
  }

  template<typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    List<NodeType, NodeTypeAlloc> lst;
    lst.emplace(lst.begin(), std::forward<Args>(args)...);
    auto find_result = find(lst.begin()->first);
    if (find_result != list_.end()) {
      return {find_result, false};
    }
    size_t hash = hashFunc_(lst.begin()->first) % buckets_.size();
    if (buckets_[hash] != list_.end()) {
      list_.emplace(buckets_[hash], std::forward<Args>(args)...);
    } else {
      list_.emplace(list_.end(), std::forward<Args>(args)...);
    }
    check_for_rehash();
    return {--buckets_[hash], true};
  }

  std::pair<iterator, bool> insert(const NodeType& node) {
    return emplace(node);
  }

  template<typename T>
  std::pair<iterator, bool> insert(T&& node) {
    return emplace(std::forward<T>(node));
  }

  template<class InputIterator>
  void insert(InputIterator first, InputIterator last) {
    while (first != last) {
      emplace(*first);
      ++first;
    }
  }

  void erase(const_iterator position) {
    size_t hash = hashFunc_(position->first) % buckets_.size();
    iterator temp = buckets_[hash];
    ++temp;
    if (buckets_[hash] == position and temp != list_.end() and hashFunc_(temp->first) % buckets_.size() == hash) {
      ++buckets_[hash];
    } else {
      buckets_[hash] = list_.end();
    }
    list_.erase(position);
  }

  void erase(const_iterator first, const_iterator last) {
    while (first != last) {
      erase(first++);
    }
  }

  float load_factor() const {
    return list_.size() / buckets_.size();
  }

  float max_load_factor() const noexcept {
    return max_load_factor_;
  }

  void max_load_factor (float new_max_load) {
    max_load_factor_ = new_max_load;
  }

  void swap(UnorderedMap& ump) {
    std::swap(alloc_, ump.alloc);
    swapMap(ump);
  }
};
