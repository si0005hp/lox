#pragma once

#include "../common.h"

namespace lox {

  template <class T, int Size>
  class Queue {
   public:
    Queue()
      : head_(0)
      , count_(0) {}

    int count() const {
      return count_;
    }

    bool isEmpty() const {
      return count_ == 0;
    }

    int capacity() const {
      return Size;
    }

    void clear() {
      while (!isEmpty()) dequeue();
    }

    void enqueue(const T& item) {
      ASSERT(count_ < Size, "Queue is full.");

      items_[head_] = item;
      head_ = wrap(head_ + 1);
      count_++;
    }

    T dequeue() {
      ASSERT(count_ > 0, "Queue is empty.");

      int tail = wrap(head_ - count_);

      T item = items_[tail];
      items_[tail] = T();

      count_--;
      return item;
    }

    T& operator[](int index) {
      ASSERT_INDEX(index, count_);

      return items_[wrap(head_ - count_ + index)];
    }

   private:
    inline int wrap(int index) const {
      return (index + Size) % Size;
    }

    int head_;
    int count_;
    T items_[Size];
  };

} // namespace lox
