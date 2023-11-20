#include <iostream>
#include <memory>
template <typename T>
class EnableSharedFromThis {

};

template<typename T>
class SharedPtr;

template <typename U, typename Alloc, typename... Args>
SharedPtr<U> allocateShared(Alloc alloc, Args&&... args) {
  using ControlBlock = typename SharedPtr<U>::template ControlBlockMakeShared<U, Alloc>;
  using ControlBlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlock>;
  using AllocTraits = typename std::allocator_traits<ControlBlockAlloc>;
  ControlBlockAlloc newAlloc = alloc;
  auto cb = AllocTraits::allocate(newAlloc, 1);
  std::allocator_traits<Alloc>::construct(alloc, cb, 1, 0, alloc, std::forward<Args>(args)...);
  return SharedPtr<U>(reinterpret_cast<typename SharedPtr<U>::BaseControlBlock*>(cb));
}

template<typename U, typename... Args>
SharedPtr<U> makeShared(Args&&... args) {
  return allocateShared<U, std::allocator<U>>(std::allocator<U>(), std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr;

template<typename T>
class SharedPtr {

  template<typename U>
  friend class WeakPtr;

  template<typename U>
  friend class SharedPtr;

  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(Alloc alloc, Args&&... args);

  template<typename U, typename... Args>
  friend SharedPtr<U> makeShared(Args&&... args);

  //=======================================================================

  struct BaseControlBlock {
    int shared_count_;
    int weak_count_;

    BaseControlBlock(int a, int b): shared_count_(a), weak_count_(b) {}

    virtual void deallocate() = 0;
    virtual void destruct() = 0;
    virtual ~BaseControlBlock() = default;
    virtual T* get_ptr() = 0;
  };

  //==========================================================================

  template<typename U, typename Alloc = std::allocator<U>>
  struct ControlBlockMakeShared : BaseControlBlock {

    U object;
    Alloc alloc = Alloc();

    using ControlBlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockMakeShared>;
    using AllocTraits = typename std::allocator_traits<ControlBlockAlloc>;

    void destruct() override {
      std::allocator_traits<Alloc>::destroy(alloc, &object);
    }

    void deallocate() override {
      ControlBlockAlloc newAlloc = alloc;
      AllocTraits::deallocate(newAlloc, this, 1);
    }

    T* get_ptr() override {
      return &object;
    }

    template<typename... Args>
    ControlBlockMakeShared(int a, int b, Alloc alloc, Args&&... args):
    BaseControlBlock(a, b), object(std::forward<Args>(args)...), alloc(alloc) {
    }
  };

  //===========================================================================

  template<typename U, typename Alloc, typename Deleter>
  struct ControlBlockRegular: BaseControlBlock {
    U* object;
    Deleter deleter;
    Alloc alloc;

    void destruct() override {
      deleter(object);
    }
    void deallocate() override {
      using ControlBlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular>;
      using AllocTraits = typename std::allocator_traits<ControlBlockAlloc>;
      ControlBlockAlloc newAlloc = alloc;
      AllocTraits::deallocate(newAlloc, this, 1);
    }

    T* get_ptr() override {
      return object;
    }

    ControlBlockRegular(int a, int b, U* ptr, Deleter deleter, Alloc alloc): BaseControlBlock(a, b), object(ptr),
                                                                             deleter(deleter), alloc(alloc) {}
  };

  //=============================================================================

  T* ptr = nullptr;
  BaseControlBlock* cb = nullptr;

  int& get_shared_count() const {
    return cb->shared_count_;
  }

  int& get_weak_count() const {
    return cb->weak_count_;
  }

  void incr_shared_count() {
    if (!cb) return;
    ++get_shared_count();
  }

  bool decr_shared_count() {
    if (cb and cb->shared_count_ != 0) {
      --get_shared_count();
      return true;
    }
    return false;
  }

  template <typename Y, typename U>
  SharedPtr(typename SharedPtr<Y>::BaseControlBlock* cb, U* ptr = nullptr): ptr(ptr), cb(cb) {}

  SharedPtr(BaseControlBlock* cb): ptr(nullptr), cb(cb) {}

  template<typename U>
  SharedPtr(const WeakPtr<U>& other): ptr(other.ptr), cb(other.cb) {
    incr_shared_count();
  }

 public:

  template<typename U, typename Deleter = std::default_delete<U>, typename Alloc = std::allocator<U>>
  explicit SharedPtr(U* ptr, Deleter deleter = Deleter(), Alloc alloc = Alloc()) : ptr(reinterpret_cast<T*>(ptr)) {

    using ControlBlock = ControlBlockRegular<U, Alloc, Deleter>;
    using ControlBlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlock>;
    using DefaultControlBlockAlloc = typename std::allocator<ControlBlock>;
    using AllocTraits = typename std::allocator_traits<ControlBlockAlloc>;
    using DefaultAllocTraits = typename std::allocator_traits<DefaultControlBlockAlloc>;
    ControlBlockAlloc newAlloc = alloc;
    DefaultControlBlockAlloc newnewAlloc;
    cb = AllocTraits::allocate(newAlloc, 1);
    DefaultAllocTraits::construct(newnewAlloc, static_cast<ControlBlock*>(cb), 1, 0, ptr, deleter, alloc);
  }

  ~SharedPtr() {
    if (!decr_shared_count()) {
      return;
    }
    if (get_shared_count() == 0) {
      cb->destruct();
      if (get_weak_count() == 0) {
        cb->deallocate();
      }
    }
  }

  SharedPtr(): ptr(nullptr), cb(nullptr) {}

  SharedPtr(const SharedPtr& other): ptr(other.ptr), cb(other.cb) {
    incr_shared_count();
  }
  template<typename U>
  SharedPtr(const SharedPtr<U>& other): ptr(other.ptr), cb(reinterpret_cast<BaseControlBlock*>(other.cb)) {
    incr_shared_count();
  }

  SharedPtr(SharedPtr&& other): ptr(other.ptr), cb(other.cb) {
    incr_shared_count();
    other.reset();
  }

  template<typename U>
  SharedPtr(SharedPtr<U>&& other): ptr(other.ptr), cb(reinterpret_cast<BaseControlBlock*>(other.cb)) {
    incr_shared_count();
    other.reset();
  }

  SharedPtr& operator=(const SharedPtr& other) {
    auto temp = other;
    swap(temp);
    return *this;
  }

  template <typename U>
  SharedPtr& operator=(const SharedPtr<U>& other) {
    auto temp = SharedPtr(other);
    swap(temp);
    return *this;
  }

  SharedPtr& operator=(SharedPtr&& other) {
    auto temp = SharedPtr(std::move(other));
    swap(temp);
    return *this;
  }

  template <typename U>
  SharedPtr& operator=(SharedPtr<U>&& other) {
    auto temp = SharedPtr(std::move(other));
    swap(temp);
    return *this;
  }

  template <typename U>
  void reset(U* pointer) {
    reset();
    auto temp = SharedPtr<T>(pointer);
    swap(temp);
  }

  void reset() {
    if (!decr_shared_count()) {
      return;
    }
    if (get_shared_count() == 0) {
      cb->destruct();
      if (get_weak_count() == 0) {
        cb->deallocate();
      }
    }
    cb = nullptr;
  }

  T* get() {
    return (cb ? cb->get_ptr() : nullptr);
  }

  const T* get() const {
    return (cb ? cb->get_ptr() : nullptr);
  }

  void swap(SharedPtr& other) {
    std::swap(ptr, other.ptr);
    std::swap(cb, other.cb);
  }

  int use_count() const {
    return get_shared_count();
  }

  T* operator->() const {
    return ptr;
  }

  T& operator*() const {
    return *ptr;
  }
};

//===========================================================================================================================================================================

template<typename T>
class WeakPtr {
 private:
  T* ptr;
  typename SharedPtr<T>::BaseControlBlock* cb;

  template<typename U>
  friend class SharedPtr;

  template<typename U>
  friend class WeakPtr;

  using SP = SharedPtr<T>;

 public:
  WeakPtr() : ptr(nullptr), cb(nullptr) {}

  template<typename U>
  WeakPtr(const SharedPtr<U>& sp) : ptr(sp.ptr), cb(reinterpret_cast<typename SP::BaseControlBlock*>(sp.cb)) {
    incr_weak_count();
  }

  WeakPtr(const WeakPtr& wp) : ptr(wp.ptr), cb(wp.cb) {
    incr_weak_count();
  }

  template<typename U>
  WeakPtr(const WeakPtr<U>& wp) : ptr(wp.ptr), cb(reinterpret_cast<typename SP::BaseControlBlock*>(wp.cb)) {
    incr_weak_count();
  }

  WeakPtr(WeakPtr&& wp) noexcept : ptr(wp.ptr), cb(wp.cb) {
    incr_weak_count();
    wp.reset();
  }

  template<typename U>
  WeakPtr(WeakPtr<U>&& wp) noexcept : ptr(wp.ptr), cb(reinterpret_cast<typename SP::BaseControlBlock*>(wp.cb)) {
    incr_weak_count();
    wp.reset();
  }

  template<typename U>
  void swap(WeakPtr<U>& other) {
    std::swap(ptr, other.ptr);
    std::swap(cb, other.cb);
  }

  ~WeakPtr() {
    reset();
  }

  WeakPtr& operator=(const WeakPtr& wp) {
    WeakPtr temp = wp;
    swap(temp);
    return *this;
  }
  template<typename U>
  WeakPtr& operator=(const WeakPtr<U>& wp) {
    WeakPtr temp = WeakPtr(wp);
    swap(temp);
    return *this;
  }

  WeakPtr& operator=(WeakPtr&& wp) {
    WeakPtr temp = std::move(wp);
    swap(temp);
    return *this;
  }
  template<typename U>
  WeakPtr& operator=(WeakPtr<U>&& wp) {
    WeakPtr temp = WeakPtr(std::move(wp));
    swap(temp);
    return *this;
  }

  bool expired() const noexcept {
    return get_shared_count() == 0;
  }

  SharedPtr<T> lock() const noexcept {
    if (expired()) {
      return SharedPtr<T>();
    }
    return SharedPtr<T>(*this);
  }

  void reset() {
    if (!decr_weak_count()) {
      return;
    }
    if (get_shared_count() == 0) {
      if (get_weak_count() == 0) {
        cb->deallocate();
      }
    }
    cb = nullptr;
  }

  int& get_shared_count() const {
    return cb->shared_count_;
  }

  int& get_weak_count() const {
    return cb->weak_count_;
  }

  void incr_weak_count() {
    if (!cb) return;
    ++get_weak_count();
  }

  bool decr_weak_count() {
    if (cb and cb->weak_count_ != 0) {
      --get_weak_count();
      return true;
    }
    return false;
  }

  int use_count() {
    return get_shared_count();
  }
};

