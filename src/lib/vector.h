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
      , items_(NULL) {}

    Vector(int capacity)
      : count_(0)
      , capacity_(0)
      , items_(NULL) {
      ensureCapacity(capacity);
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
      Memory::deallocate(items_);
      count_ = 0;
      capacity_ = 0;
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
      return const_cast<T&>(get(index));
    }

    const T& operator[](int index) const {
      return get(index);
    }

  private:
    const T& get(int index) const {
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
