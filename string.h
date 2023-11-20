#include <algorithm>
#include <cstring>
#include <iostream>
class String {
  private:
  size_t capacity_;
  size_t size_;
  char* data_;
  void reallocate(size_t size) {
    char *new_data = new char[size];
    capacity_ = size;
    std::copy(data_, data_ + size_, new_data);
    delete[] data_;
    data_ = new_data;
  }
  void swap_str(String& str) {
    std::swap(data_, str.data_);
    std::swap(capacity_, str.capacity_);
    std::swap(size_, str.size_);
  }
  public:
  String() : capacity_(0), size_(0), data_(new char[0]) {}
  String(const String& str) : capacity_(str.size_), size_(str.size_), data_(new char[capacity_]) {
    std::copy(str.data_, str.data_ + size_, data_);
  }
  String(size_t size, char chr): capacity_(size), size_(size), data_(new char[capacity_]) {
    std::fill(data_, data_ + size, chr);
  }
  String(const char* str): capacity_(strlen(str)), size_(capacity_), data_(new char[capacity_]) {
    std::copy(str, str + size_ + 1, data_);
  }
  ~String() { delete[] data_; }
  String& operator=(String str) {
    swap_str(str);
    return *this;
  }
  size_t length() const { return size_; }
  size_t capacity() const { return capacity_; }
  size_t size() const { return size_; }
  char& operator[](size_t i) { return data_[i]; }
  const char& operator[](size_t i) const { return data_[i]; }
  void pop_back() { --size_; }
  void push_back(char a) {
    if (size_ + 1 > capacity_) {
      reallocate(2 * capacity_);
    }
    data_[size_++] = a;
  }
  const char& front() const { return data_[0]; }
  const char& back() const { return data_[size_ - 1]; }
  char& front() { return data_[0]; }
  char& back() { return data_[size_ - 1]; }
  String& operator+=(char a) {
    push_back(a);
    return *this;
  }
  String& operator+=(const String& str) {
    if (size_ + str.size_ > capacity_) {
      reallocate(size_ + str.size_);
      std::copy(str.data_, str.data_ + str.size_, data_ + size_);
      size_ += str.size_;
    }
    else {
      std::copy(str.data_, str.data_ + str.size_, data_ + size_);
      size_ += str.size_;
    }
    return *this;
  }
  String substr(size_t start, size_t count) const {
    String temp(count, '.');
    std::copy(data_ + start, data_ + start + count, temp.data_);
    return temp;
  }
  size_t find(const String& str) const {
    for (size_t i = 0; i <= size_ - str.size_; ++i) {
      bool flag = memcmp(str.data_, data_ + i, str.size());
      if (!flag) return i;
    }
    return length();
  }
  size_t rfind(const String& str) const {
    for (size_t i = size_ - str.size_ + 1; i > 0; --i) {
      bool flag = memcmp(str.data_, data_ + i - 1, str.size());
      if (!flag) return --i;
    }
    return length();
  }
  bool empty() const { return size_ == 0; }
  void clear() { size_ = 0; }
  void shrink_to_fit() {
    reallocate(size_);
  }
  char* data() const { return data_; };
};
bool operator<(const String& a, const String& b) {
  return a.size() < b.size() || ((a.size() == b.size()) && memcmp(a.data(), b.data(), a.size()) < 0);
}
bool operator==(const String& a, const String& b) {
  if (a.size() != b.size())
    return false;
  return !memcmp(a.data(), b.data(), a.size());
}
bool operator>=(const String& a, const String& b) {
  return !(a < b);
}
bool operator<=(const String& a, const String& b) {
  return !(b < a);
}
bool operator>(const String& a, const String& b) {
  return (b < a);
}
bool operator!=(const String& a, const String& b) {
  return !(a == b);
}
String operator+(String a, const String& b) {
  a += b;
  return a;
}
String operator+(String a, char b) {
  a.push_back(b);
  return a;
}
String operator+(char a, const String& b) {
  String temp;
  temp += a;
  temp += b;
  return temp;
}
std::ostream& operator<<(std::ostream &out, const String& str) {
  for (size_t i = 0; i < str.length(); ++i) {
    out << str[i];
  }
  return out;
}
std::istream& operator>>(std::istream &in, String& str) {
  char temp;
  while (in.get(temp) && !std::isspace(temp)) {
    str.push_back(temp);
  }
  return in;
}

