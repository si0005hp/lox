#pragma once

#include "../common.h"
#include "../memory.h"

namespace lox {

  // TODO:
  template <class K, class V>
  class Map {
   public:
    Map()
      : count_(0)
      , capacity_(0)
      , entries_(nullptr) {}

    ~Map() {
      clear();
    }

    bool get(const K& key, V* value) const {
      int index = findIndex(key);

      if (index == -1) return false;

      *value = entries_[index].value;
      return true;
    }

    void put(const K& key, const V& value) {
      count_++;
      ensureCapacity();

      int index = static_cast<int>(key.hashCode() & 0x7fffffff) % capacity_;

      while (!(entries_[index].key == K()) && !(entries_[index].key == key)) {
        index = (index + 1) % capacity_;
      }

      entries_[index].key = key;
      entries_[index].value = value;
    }

    bool remove(const K& key) {
      int index = findIndex(key);

      if (index == -1) return false;

      entries_[index].key = K();
      entries_[index].value = V();

      count_--;
      return true;
    }

    void clear() {
      Memory::deallocate(entries_);
      count_ = 0;
      capacity_ = 0;
    }

    bool containsKey(const K& key) {
      return findIndex(key) != -1;
    }

   private:
    struct Entry {
      K key;
      V value;
    };

    int findIndex(const K& key) const {
      if (capacity_ == 0) return -1;

      int index = static_cast<int>(key.hashCode() & 0x7fffffff) % capacity_;

      while (true) {
        if (entries_[index].key == key) return index;

        if (entries_[index].key == K()) return -1;

        index = (index + 1) % capacity_;
      }
    }

    void ensureCapacity() {
      if (count_ <= capacity_ * MAX_LOAD_PERCENT / 100) return;

      int oldSize = capacity_;
      capacity_ = MIN_CAPACITY;
      if (oldSize >= MIN_CAPACITY) {
        capacity_ = oldSize * GROW_FACTOR;
      }

      Entry* oldEntries = entries_;

      void* mem = Memory::reallocate(nullptr, 0, sizeof(Entry) * capacity_);
      entries_ = ::new (mem) Entry[capacity_];

      if (oldEntries != nullptr) {
        for (int i = 0; i < oldSize; i++) {
          if (!(oldEntries[i].key == K())) put(oldEntries[i].key, oldEntries[i].value);
        }
        Memory::deallocate(oldEntries);
      }
    }

    static constexpr int MAX_LOAD_PERCENT = 75;
    static constexpr int MIN_CAPACITY = 16;
    static constexpr int GROW_FACTOR = 2;

    int count_;
    int capacity_;
    Entry* entries_;
  };

} // namespace lox
