#include <exception>
#include <iostream>
#include <memory>
#include <vector>
#include <iostream>


template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:

  struct BaseNode {
    BaseNode* prev;
    BaseNode* next;
    BaseNode(): prev(nullptr), next(nullptr) {}
    BaseNode(BaseNode* first, BaseNode* second): prev(first), next(second) {}
  };

  struct Node: public BaseNode {
    T value;

    Node(): BaseNode(nullptr, nullptr) {};
    Node(BaseNode* left, BaseNode* right, const T& value): BaseNode(left, right), value(value) {}
    Node(const T& value): value(value) {}
  };

 public:
  typedef typename std::allocator_traits<Alloc>::template rebind_alloc<Node> NodeAllocator;

 private:
  //attributes
  BaseNode fakeNode_;
  size_t size_;
  NodeAllocator alloc_;

  void clean_up(size_t sz)  {
    BaseNode* curr = fakeNode_.next;
    for (size_t i = 0; i < sz; ++i) {
      BaseNode* temp;
      temp = curr->next;
      AllocTraits::destroy(alloc_, reinterpret_cast<Node*>(curr));
      AllocTraits::deallocate(alloc_, reinterpret_cast<Node*>(curr), 1);
      curr = temp;
    }
    size_ = 0;
  }

 public:
  template<typename V>
  struct base_iterator {
   public:
    using difference_type = ptrdiff_t;
    using value_type = V;
    using pointer = V*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

   private:
    BaseNode* curr_;
    void swap_iter(base_iterator& it) { std::swap(curr_, it.curr_); }

   public:

    base_iterator (const BaseNode* node) : curr_(const_cast<BaseNode*>(node)){}

    base_iterator (BaseNode* node) : curr_(node) {}

    base_iterator (const base_iterator& it) : curr_(it.curr_) {}

    value_type& operator*() {
      return reinterpret_cast<Node*>(curr_)->value;
    }

    value_type* operator->() {
      return &(reinterpret_cast<Node*>(curr_)->value);
    }

    base_iterator& operator=(base_iterator it) {
      swap_iter(it);
      return *this;
    }
    base_iterator& operator ++() {
      *this = base_iterator(curr_->next);
      return *this;
    }

    base_iterator operator ++(int) {
      auto temp  = *this;
      ++*this;
      return temp;
    }

    base_iterator operator --() {
      return *this = base_iterator(curr_->prev);
    }

    base_iterator operator --(int) {
      auto temp = *this;
      --*this;
      return temp;
    }

    BaseNode* get_node_ptr() const {
      return curr_;
    }

    bool operator ==(const base_iterator& b) const {
      return curr_ == b.curr_;
    }
    bool operator != (const base_iterator& b) const {
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

  List(const Alloc& allocator): fakeNode_(&fakeNode_, &fakeNode_), size_(0), alloc_(allocator) {}

  List& operator=(const List& other) {
    List temp = other;
    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }
    std::swap(fakeNode_, temp.fakeNode_);
    std::swap(size_, temp.size_);
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

  void insert(const_iterator it, const T& value) {
    BaseNode* right = it.get_node_ptr();
    BaseNode* left = right->prev;
    BaseNode *to_be_inserted;
    try {
      to_be_inserted = AllocTraits::allocate(alloc_, 1);
      AllocTraits::construct(alloc_, reinterpret_cast<Node*>(to_be_inserted), left, right, value);
    } catch(...) {
      throw;
    }
    right->prev = to_be_inserted;
    left->next = to_be_inserted;
    ++size_;
  }

  void erase(const_iterator it) {
    BaseNode* to_be_erased = it.get_node_ptr();
    BaseNode* left = to_be_erased->prev;
    BaseNode* right = to_be_erased->next;
    left->next = right;
    right->prev = left;
    AllocTraits::destroy(alloc_, reinterpret_cast<Node*>(to_be_erased));
    AllocTraits::deallocate(alloc_, reinterpret_cast<Node*>(to_be_erased), 1);
    --size_;
  }

  NodeAllocator get_allocator() {
    return alloc_;
  }
  size_t size() const {
    return size_;
  }
  void push_back(const T &value) {
    insert( cend(), value);
  }
  void push_front(const T &value) {
    insert(cbegin(), value);
  }
  void pop_front() {
    erase(cbegin());
  }
  void pop_back() {
    erase(--cend());
  }

  ~List() {
    clean_up(size_);
  }

  List(size_t size, Alloc allocator = Alloc()) : fakeNode_(&fakeNode_, &fakeNode_), alloc_(allocator) {
    BaseNode* previous = &fakeNode_;
    Node* curr;
    size_ = 0;
    for (size_t i = 0; i < size; ++i) {
      try {
        curr = AllocTraits::allocate(alloc_, 1);
      } catch (...) {
        clean_up(size_);
        throw;
      }
      try {
        AllocTraits::construct(alloc_, curr);
      } catch (...) {
        AllocTraits::deallocate(alloc_, curr, 1);
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

  List(size_t size, const T& value, Alloc allocator = Alloc()) : fakeNode_(&fakeNode_, &fakeNode_), size_(size), alloc_(allocator) {
    BaseNode* previous = &fakeNode_;
    Node* curr;
    for (size_t i = 0; i < size_; ++i) {
      try {
        curr = AllocTraits::allocate(alloc_, 1);
      } catch (...) {
        clean_up();
        throw;
      }
      try {
        AllocTraits::construct(alloc_, curr, value);
      } catch (...) {
        AllocTraits::deallocate(alloc_, curr, 1);
        clean_up();
        throw;
      }
      curr->prev = previous;
      previous->next = curr;
      previous = curr;
    }
    fakeNode_.prev = curr;
    curr->next = &fakeNode_;
  }

  List(const List& other): List(AllocTraits::select_on_container_copy_construction(other.alloc_)){
    auto it = other.begin();
    size_ = 0;
    for (size_t i = 0; i < other.size(); ++i) {
      try {
        push_back(*it);
        ++it;
      } catch(...) {
        clean_up(size_);
        throw;
      }
    }
  }
};

template <size_t N>
class StackStorage {
 public:
  int8_t arr_[N];
  size_t occupied_ = 0;

 public:
  StackStorage() = default;
  ~StackStorage() = default;
  StackStorage(const StackStorage& storage) = delete;
};

template <typename T, size_t N>
class StackAllocator {
 public:
  StackStorage<N>* storage_;
  using value_type = T;

  template<typename U>
  struct rebind {
    typedef U value_type;
    typedef StackAllocator<U, N> other;
  };
  StackAllocator() : storage_(nullptr) {}
  StackAllocator(StackStorage<N>& store): storage_(&store) {}

  template<typename U>
  StackAllocator(const StackAllocator<U, N>& other) : storage_(other.storage_) {}

  template<typename U>
  StackAllocator<T, N>& operator=(const StackAllocator<U, N>& other) {
    storage_ = other.storage_;
    return *this;
  }

  bool operator ==(const StackAllocator<T, N>& other) {
    return storage_ == other.storage_;
  }
  bool operator != (const StackAllocator& other) {
    return !(*this == other);
  }
  value_type* allocate(size_t n) {
    void* aligned_ptr = storage_->arr_ + storage_->occupied_;
    size_t space = N - storage_->occupied_;
    if (std::align(alignof(value_type), n * sizeof(value_type),aligned_ptr , space)) {
      storage_->occupied_ = N - space + n * sizeof(value_type);
      return reinterpret_cast<value_type *>(aligned_ptr);
    } else {
      throw std::bad_alloc();
    }
  }
  void deallocate(void* ptr, size_t) { std::ignore = ptr; }
};
