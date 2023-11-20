#include <algorithm>
#include <vector>
#include <iostream>
#include <cmath>
#include <exception>
#include <stdexcept>
struct Node {
  size_t bucket;
  size_t index;
};

template <typename T>
class Deque {
 private:
  mutable std::vector<T*> external_;
  static const size_t bucket_sz_ = 32;
  size_t cap_;
  size_t sz_;
  Node first_;
  Node last_;

  void reserve_(size_t newcap);
  void swap_deque(Deque& d) {
    std::swap(external_, d.external_);
    std::swap(cap_, d.cap_);
    std::swap(sz_, d.sz_);
    std::swap(first_, d.first_);
    std::swap(last_, d.last_);
  }

  template <typename U>
  void free(U& arr, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
      delete[] reinterpret_cast<uint8_t*>(arr[i]);
    }
  }

  template<typename U>
  void alloc(U& arr, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
      arr[i] = reinterpret_cast<T*>(new uint8_t[bucket_sz_ * sizeof(T)]);
    }
  }

  template<typename U>
  void alloc(U& arr, size_t& start, size_t end, int) {
    for (size_t i = start; i < end; ++i) {
      arr[i] = reinterpret_cast<T*>(new uint8_t[bucket_sz_ * sizeof(T)]);
    }
  }

  template <typename U>
  void index_alloc(U& arr, size_t index) {
    alloc(arr, index, index + 1);
  }


  template <typename U>
  void index_free(U& arr, size_t index) {
    free(arr, index, index + 1);
  }

  template <typename U>
  void destruct_range(U& arr, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
      (arr + i)->~T();
    }
  }

  void destruct_self(size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
      (&operator[](i))->~T();
    }
  }
 public:
  Deque();
  explicit Deque (size_t count, const T& v);
  explicit Deque (int sizee);
  Deque (const Deque& d);
  ~Deque ();
  size_t size() const;
  size_t capacity() const;
  const T& operator[](size_t i) const;
  T& operator[](size_t i);
  const T& at(size_t i) const;
  T& at(size_t i);
  void push_back(const T& a);
  void push_front(const T& a);
  void pop_front();
  void pop_back();

  template<typename V>
  struct base_iterator {
   public:
    using difference_type = ptrdiff_t;
    using value_type = V;
    using pointer = V*;
    using reference = value_type&;
    using iterator_category = std::random_access_iterator_tag;

   private:
    T** out_;
    T* in_;
    void swap_iter(base_iterator& it) {
      std::swap(in_, it.in_);
      std::swap(out_, it.out_);
    }

   public:

    base_iterator (T** out, T* in) : out_(out), in_(in) {
    }

    base_iterator (const base_iterator& it) : out_(it.out_), in_(it.in_) {}

    value_type& operator*() {
      return *in_;
    }

    value_type* operator->() {
      return in_;
    }

    base_iterator& operator=(base_iterator it) {
      swap_iter(it);
      return *this;
    }
    base_iterator& operator ++() {
      if (in_ == *out_ + bucket_sz_ - 1) {
        --out_;
        in_ = *out_;
      } else {
        ++in_;
      }
      return *this;
    }

    base_iterator operator ++(int) {
      auto temp  = *this;
      ++*this;
      return temp;
    }

    base_iterator operator --() {
      if (in_ == *out_) {
         ++out_;
         in_ = *out_ + bucket_sz_ - 1;
      } else {
        --in_;
      }
      return *this;
    }

    base_iterator operator --(int) {
      auto temp = *this;
      --*this;
      return temp;
    }

    base_iterator& operator+=(difference_type diff) {
      if (diff < 0) {
        return *this -= (-diff);
      }
      size_t shift = diff / bucket_sz_;
      diff %= bucket_sz_;
      auto out1 = out_;
      out_ -= shift;
      if (in_ + diff > *out1 + bucket_sz_ - 1) {
        size_t temp = in_ + diff - (*out1 + bucket_sz_ - 1);
        --out_;
        in_ = *out_ + temp - 1;
        return *this;
      }
      in_ = *out_ +  (in_ - *out1) + diff;
      return *this;
    }

    base_iterator& operator-=(difference_type diff) {
      if (diff < 0) {
        return *this += (-diff);
      }
      size_t shift = diff / bucket_sz_;
      diff %= bucket_sz_;
      auto out1 = out_;
      out_ += shift;
      if (in_ < *out1 + diff) {
        size_t temp = (*out1 + diff) - in_;
        ++out_;
        in_ = *out_ + bucket_sz_ - temp;
      } else {
        in_ = *out_ + (in_ - *out1) - diff;
      }
      return *this;
    }

    base_iterator operator+(difference_type diff) const {
      auto temp = *this;
      temp += diff;
      return temp;
    }

    base_iterator operator-(difference_type diff) const {
      auto temp = *this;
      temp -= diff;
      return temp;
    }

    difference_type operator-(const base_iterator& b) const {
      if (b > *this) {
        return (b - *this);
      }
      if (out_ == b.out_) {
        return in_ - b.in_;
      }
      return (b.out_ - out_ - 1) * bucket_sz_ + (in_ - *out_) + (*b.out_ + bucket_sz_ - 1 - b.in_) + 1;
    }

    bool operator ==(const base_iterator& b) const {
      return b.out_ == out_ and b.in_ == in_;
    }
    bool operator != (const base_iterator& b) const {
      return !(*this == b);
    }
    bool operator <(const base_iterator& b) const {
      return b.out_ < out_ or (b.out_ == out_ and b.in_ > in_);
    }
    bool operator >(const base_iterator& b) const {
      return b < *this;
    }
    bool operator >=(const base_iterator& b) const {
      return !(b > *this);
    }
    bool operator <=(const base_iterator& b) const {
      return b >= *this;
    }
    operator base_iterator<const V>() const {
      return base_iterator<const V>(out_, in_);
    }
  };

  using iterator = base_iterator<T>;
  using const_iterator = base_iterator<const T>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque& operator=(Deque d) {
    swap_deque(d);
    return *this;
  }

  void insert(iterator it, const T& value) {
    auto diff = it - begin();
    if (last_.bucket == 0 and last_.index == bucket_sz_ - 1) {
      reserve_(std::max(2 *  cap_, (size_t)4));
    }
    it = begin() + diff;
    if ((size_t)diff == sz_) {
      push_back(value);
      return;
    }
    auto it1 = end();
    new(it1.operator->()) T(*(it1 - 1));
    --it1;
    while (it1 != it) {
      *(it1) = *(it1 - 1);
      --it1;
    }
    *it = value;
    ++sz_;
    if (last_.index == bucket_sz_ - 1) {
      --last_.bucket;
      last_.index = 0;
    } else {
      ++last_.index;
    }
  }

  void erase(iterator it) {
    auto it1 = it;
    while (it1 < end() - 1) {
      *(it1) = *(it1 + 1);
      ++it1;
    }
    (it1.operator->())->~T();
    --sz_;
    if (last_.index == 0) {
      ++last_.bucket;
      last_.index = bucket_sz_ - 1;
    } else {
      --last_.index;
    }
  }

  iterator begin() {
    return {&external_[first_.bucket], &external_[first_.bucket][first_.index]};
  }

  iterator end() {
    if (sz_ == 0) {
      return {&external_[last_.bucket], &external_[last_.bucket][last_.index]};
    }
    if (last_.index == bucket_sz_ - 1) {
      return {external_.data() + last_.bucket - 1, &(*external_[last_.bucket - 1])};
    }
    return {&external_[last_.bucket], &external_[last_.bucket][last_.index + 1]};
  }

  const_iterator begin() const {
    return cbegin();
  }

  const_iterator end() const {
    return cend();
  }

  const_iterator cbegin() {
    return const_iterator(&external_[first_.bucket], &external_[first_.bucket][first_.index]);
  }

  const_iterator cend() {
    if (sz_ == 0) {
      return {&external_[last_.bucket], &external_[last_.bucket][last_.index]};
    }
    if (last_.index == bucket_sz_ - 1) {
      return const_iterator(&external_[last_.bucket - 1], external_[last_.bucket - 1]);
    }
    return const_iterator(&external_[last_.bucket], &external_[last_.bucket][last_.index + 1]);
  }


  const_iterator cbegin() const {
    return const_iterator(&external_[first_.bucket], &external_[first_.bucket][first_.index]);
  }

  const_iterator cend() const {
    if (sz_ == 0) {
      return {&external_[last_.bucket], &external_[last_.bucket][last_.index]};
    }
    if (last_.index == bucket_sz_ - 1) {
      return const_iterator(&external_[last_.bucket - 1], external_[last_.bucket - 1]);
    }
    return const_iterator(&external_[last_.bucket], &external_[last_.bucket][last_.index + 1]);
  }

  reverse_iterator rbegin() {
    return std::reverse_iterator<iterator>(end());
  }

  reverse_iterator rend() {
    return std::reverse_iterator<iterator>(begin());
  }

  reverse_iterator rbegin() const {
    return std::reverse_iterator<iterator>(end());
  }

  reverse_iterator rend() const {
    return std::reverse_iterator<iterator>(begin());
  }

  const_reverse_iterator crbegin() const {
    return std::reverse_iterator<iterator>(end());
  }

  const_reverse_iterator crend() const {
    return std::reverse_iterator<iterator>(begin());
  }

  const_reverse_iterator crbegin() {
    return std::reverse_iterator<iterator>(end());
  }

  const_reverse_iterator crend() {
    return std::reverse_iterator<iterator>(begin());
  }
};

template<typename T>
void Deque<T>::reserve_(size_t newcap) {
  if (cap_ == 0) {
    std::vector<T*> temp(4);
    size_t i = 0;
    try {
      alloc(temp, i, 4, 1);
    } catch (...) {
      free(temp, 0, i);
      throw;
    }
    cap_ = 4;
    external_ = temp;
    first_ = {2, bucket_sz_};
    last_ = {1, 0};
    sz_ = 0;
  }
  if (newcap <= cap_) {
    return;
  }
  std::vector<T*> temp(newcap);

  size_t shift_top = (newcap - (first_.bucket - last_.bucket + 1)) / 2;
  size_t shift_bottom = newcap - (first_.bucket - last_.bucket + 1) - shift_top;
  size_t i = 0;
  try {
    for (; i < newcap; ++i) {
      if (shift_bottom <= i and i < newcap - shift_top) {
        temp[i] = external_[i - shift_bottom + last_.bucket];
      } else {
        index_alloc(temp, i);
      }
    }
  } catch (...) {
    for (size_t j = 0; j < i; ++j) {
      if (!(shift_bottom <= j and j < newcap - shift_top)) {
        index_free(temp, j);
      }
    }
    throw;
  }
  last_.bucket = shift_bottom;
  first_.bucket = newcap - shift_top - 1;
  cap_ = newcap;
  external_ = temp;
}
template <typename T>
Deque<T>::~Deque() {
  destruct_self(0, sz_);
  free(external_, 0, cap_);
}
template <typename T>
Deque<T>::Deque(): cap_(1), sz_(0), first_{0, 0}, last_{0, 0} {
  try {
    external_.resize(1);
    index_alloc(external_, 0);
  } catch(...) {
    index_free(external_, 0);
    throw;
  }
}

template <typename T>
Deque<T>::Deque(int sizee) {
  try {
    if (sizee == 0) {
      external_.resize(1);
      try {
        index_alloc(external_, 0);
      } catch (...) {
        throw;
      }
      cap_ = 1;
      sz_ = 0;
      first_ = {0, 0};
      last_ = {0, 0};
      return;
    }
    external_.resize(sizee / bucket_sz_ + 1);
    sz_ = sizee;
    cap_ = external_.size();
    if (sizee % bucket_sz_ == 0) {
      first_ = {cap_ - 2, 0};
    } else {
      first_ = {cap_ - 1, bucket_sz_ - (sizee % bucket_sz_)};
      last_ = {0, bucket_sz_ - 1};
    }
    size_t i = 0;
    try {
      for (; i < external_.size(); ++i) {
        index_alloc(external_, i);
      }
    } catch (...) {
      free(external_, 0, i);
      sz_ = 0;
      throw;
    }
    i = 0;
    try {
      for (; i < sz_; ++i) {
        new(&(operator[](i))) T();
      }
    } catch (...) {
      destruct_self(0, i);
      free(external_, 0, external_.size());
      sz_ = 0;
      throw;
    }
  } catch(...) {
    cap_ = 0;
    sz_ = 0;
    first_ = {0, 0};
    last_ = {0, 0};
    external_ = {};
  }
}

template <typename T>
Deque<T>::Deque(size_t count, const T& value) {
  if (count == 0) {
    cap_ = 0;
    sz_ = count;
    first_ = {0, 0};
    last_ = {0, 0};
    return;
  }
  std::vector<T*> temp(count / bucket_sz_ + 1);
  size_t i = 0, j;
  try {
    for (; i < temp.size(); ++i) {
      index_alloc(temp, i);
      j = bucket_sz_ + 1;
      try {
        if (i == temp.size() - 1) {
          for (j = 0; j < count % bucket_sz_; ++j) {
            new (temp[i] + bucket_sz_ - 1 - j) T(value);
          }
        } else {
          for (j = 0; j < bucket_sz_; ++j) {
            new(temp[i] + j) T(value);
          }
        }
      } catch(...) {
        if (i == temp.size() - 1) {
          auto ptr = temp[i] + bucket_sz_ - j;
          destruct_range(ptr, 0, j);
        } else {
          destruct_range(temp[i], 0, j);
        }
        for (size_t k = 0; k < i - 1; ++k) {
          destruct_range(temp[k], 0, bucket_sz_);
        }
        throw;
      }
    }
  } catch (...) {
    free(temp, 0, i + 1);
    if (j != bucket_sz_ + 1) {
      index_free(temp, i);
    }
    throw;
  }
  external_ = temp;
  cap_ = count / bucket_sz_ + 1;
  sz_ = count;
  if (count % bucket_sz_ == 0) {
    first_ = {cap_ - 2, 0};
  } else {
    first_ = {cap_ - 1, bucket_sz_ - (count % bucket_sz_)};
  }
  last_ = {0, bucket_sz_ - 1};
}

template <typename T>
Deque<T>::Deque(const Deque& d) {
  size_t i = 0, j = 0;
  std::vector<T*> temp(d.cap_);
  try {
    for (i = 0; i < d.cap_; ++i) {
      index_alloc(temp, i);
    }
  } catch(...) {
    free(temp, 0, i);
    throw;
  }
  i = 0;
  if (d.sz_ != 0) {
    try {
      for (i = d.last_.bucket; i <= d.first_.bucket; ++i) {
        j = 0;
        for (; j < bucket_sz_; ++j) {
          if (i == d.first_.bucket and j < d.first_.index) {
            continue;
          }
          if (i == d.last_.bucket and j > d.last_.index) {
            break;
          }
          new(temp[i] + j) T(d.external_[i][j]);
        }
      }
    } catch (...) {
      if (i == d.first_.bucket) {
        destruct_range(temp[i], d.first_.index, j);
      }
      for (size_t k = d.last_.bucket; k < i; ++k) {
        for (size_t l = 0; l < bucket_sz_; ++l) {
          if (k == d.last_.bucket and l > d.last_.index) {
            break;
          }
          (temp[k] + l)->~T();
        }
      }
      free(temp, 0, d.cap_ + 1);
      throw;
    }
  }
  external_ = temp;
  cap_ = d.cap_;
  sz_ = d.sz_;
  first_ = d.first_;
  last_ = d.last_;
}

template <typename T>
size_t Deque<T>::size() const {
  return sz_;
}

template <typename T>
size_t Deque<T>::capacity() const {
  return cap_ * bucket_sz_;
}

template <typename T>
const T& Deque<T>::operator[](size_t i) const {
  if (i + 1 > bucket_sz_ - first_.index) {
    i -= (bucket_sz_ - first_.index);
    return external_[first_.bucket - i / bucket_sz_ - 1][i % bucket_sz_];
  }
  return external_[first_.bucket][first_.index + i];
}

template <typename T>
T& Deque<T>::operator[](size_t i){
  if (i + 1 > bucket_sz_ - first_.index) {
    i -= (bucket_sz_ - first_.index);
    return external_[first_.bucket - i / bucket_sz_ - 1][i % bucket_sz_];
  }
  return external_[first_.bucket][first_.index + i];
}

template <typename T>
const T& Deque<T>::at(size_t i) const{
  if (i == sz_ or i > sz_) {
    throw std::out_of_range("exception");
  } else {
    return (*this)[i];
  }
}

template <typename T>
T& Deque<T>::at(size_t i) {
  if (i == sz_ or i > sz_) {
    throw std::out_of_range("exception");
  } else {
    return (*this)[i];
  }
}

template <typename T>
void Deque<T>::push_back(const T& a) {
  if (cap_ == 0) {
    try {
      Deque<T> temp(1, a);
      *this = temp;
    } catch (...) {
      throw;
    }
    return;
  }
  if (sz_ == 0) {
    try {
      new (external_[first_.bucket] + first_.index) T(a);
      last_.index = first_.index;
      last_.bucket = first_.bucket;
      ++sz_;
    } catch(...) {
      throw;
    }
    return;
  }
  if (last_.bucket == 0 and last_.index + 1 == bucket_sz_) {
    T* newarr = reinterpret_cast<T*>(new uint8_t[bucket_sz_ * sizeof(T)]);
    try {
      new (newarr) T(a);
    } catch(...) {
      delete[] reinterpret_cast<uint8_t*>(newarr);
      throw;
    }
    reserve_(std::max((int)cap_ * 2, 4));
    --last_.bucket;
    last_.index = 0;
    index_free(external_, last_.bucket);
    external_[last_.bucket] = newarr;
    ++sz_;
    return;
  } else {
    size_t temp = last_.index + 1;
    if (last_.index + 1 == bucket_sz_) {
      --last_.bucket;
      temp = 0;
    }
    if (sz_ == 0) {
      temp = 0;
    }
    try {
      new (external_[last_.bucket] + temp) T(a);
    } catch(...) {
      if (temp != last_.index + 1) {
        ++last_.bucket;
        last_.index = bucket_sz_ - 1;
      }
      throw;
    }
    last_.index = temp;
    if (sz_ == 0) {

    }
    ++sz_;
  }
}

template <typename T>
void Deque<T>::push_front(const T& a) {
  if (cap_ == 0) {
    try {
      Deque<T> temp(1, a);
      *this = temp;
    } catch (...) {
      throw;
    }
    return;
  }
  if (sz_ == 0) {
    try {
      new (external_[first_.bucket] + first_.index) T(a);
      last_.index = first_.index;
      last_.bucket = first_.bucket;
      ++sz_;
    } catch(...) {
      throw;
    }
    return;
  }
  if (first_.bucket == cap_ - 1 and first_.index == 0) {
    T* newarr = reinterpret_cast<T*>(new uint8_t[bucket_sz_ * sizeof(T)]);
    try {
      new (newarr + bucket_sz_ - 1) T(a);
    } catch(...) {
      delete[] reinterpret_cast<uint8_t*>(newarr);
      throw;
    }
    reserve_(std::max((int)cap_ * 2, 4));
    ++first_.bucket;
    first_.index = bucket_sz_ - 1;
    index_alloc(external_, first_.bucket);
    external_[first_.bucket] = newarr;
    ++sz_;
    return;
  } else {
    size_t temp;
    if (first_.index == 0) {
      ++first_.bucket;
      temp = bucket_sz_ - 1;
    } else {
      temp = first_.index - 1;
    }
    try {
      new (external_[first_.bucket] + temp) T(a);
    } catch(...) {
      if (temp == bucket_sz_ - 1) {
        --first_.bucket;
        first_.index = 0;
      }
      throw;
    }
    first_.index = temp;
    ++sz_;
  }
}

template <typename T>
void Deque<T>::pop_back() {
  (external_[last_.bucket] + last_.index)->~T();
  --sz_;
  if (last_.index == 0 and first_.bucket != last_.bucket) {
    ++last_.bucket;
    last_.index = bucket_sz_ - 1;
  } else if (last_.index != 0) {
    --last_.index;
  }
  if (sz_ == 0) {
    first_.index = 0;
    last_.index = 0;
  }
}

template <typename T>
void Deque<T>::pop_front() {
  (external_[first_.bucket] + first_.index)->~T();
  --sz_;
  if (first_.index == bucket_sz_ - 1 and first_.bucket != last_.bucket) {
    --first_.bucket;
    first_.index = 0;
  } else if (first_.index != bucket_sz_ - 1) {
    ++first_.index;
  }
  if (sz_ == 0) {
    first_.index = 0;
    last_.index = 0;
  }
}
