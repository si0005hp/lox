#pragma once

#include <utility>

#include "../common.h"
#include "../memory.h"

namespace lox {

  template <class T>
  class Vector {
   public:
    Vector()
      : count_(0)
      , capacity_(0)
      , items_(nullptr) {}

    Vector(int capacity)
      : count_(0)
      , capacity_(0)
      , items_(nullptr) {
      ensureCapacity(capacity);
    }

    Vector(const Vector<T>& vec)
      : count_(0)
      , capacity_(0)
      , items_(nullptr) {
      pushAll(vec);
    }

    Vector(std::initializer_list<T> items)
      : count_(0)
      , capacity_(0)
      , items_(nullptr) {
      ensureCapacity(items.size());
      for (auto item : items) items_[count_++] = item;
    }

    ~Vector() {
      clear();
    }

    void push(const T& value) {
      ensureCapacity(count_ + 1);
      items_[count_++] = value;
    }

    template <typename... Args>
    void emplace(Args&&... args) {
      ensureCapacity(count_ + 1);
      new (&items_[count_++]) T(std::forward<Args>(args)...);
    }

    void clear() {
      Memory::reallocate(items_, sizeof(T) * count_, 0); // TODO: Fix oldSize spec
      count_ = 0;
      capacity_ = 0;
    }

    void pushAll(const Vector<T>& vec) {
      ensureCapacity(count_ + vec.count_);
      for (int i = 0; i < vec.count_; i++) items_[count_++] = vec[i];
    }

    T removeAt(int index) {
      index = absIndex(index);
      ASSERT_INDEX(index, count_);

      T item = items_[index];

      // Shift items up.
      for (int i = index; i < count_ - 1; i++) {
        items_[i] = items_[i + 1];
      }

      items_[count_ - 1] = T();
      count_--;

      return item;
    }

    int size() const {
      return count_;
    }

    bool isEmpty() const {
      return count_ == 0;
    }

    T& operator[](int index) {
      return const_cast<T&>(subscript(index));
    }

    const T& operator[](int index) const {
      return subscript(index);
    }

    Vector& operator=(const Vector& other) {
      if (&other == this) return *this;

      clear();
      pushAll(other);

      return *this;
    }

   private:
    const T& subscript(int index) const {
      index = absIndex(index);
      ASSERT_INDEX(index, count_);

      return items_[index];
    }

    void ensureCapacity(int desiredCapacity) {
      if (capacity_ >= desiredCapacity) return;

      int newCapacity = capacity_ < MIN_CAPACITY ? MIN_CAPACITY : capacity_;

      while (newCapacity < desiredCapacity) {
        newCapacity *= GROW_FACTOR;
      }

      items_ = Memory::reallocate<T>(items_, sizeof(T) * capacity_, sizeof(T) * newCapacity);
      capacity_ = newCapacity;
    }

    int absIndex(int index) const {
      return index >= 0 ? index : count_ + index;
    }

    static constexpr int MIN_CAPACITY = 16;
    static constexpr int GROW_FACTOR = 2;

    int count_;
    int capacity_;
    T* items_;
  };

} // namespace lox
