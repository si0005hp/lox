#pragma once

#include "../common.h"
#include "../memory.h"

namespace lox {

  // TODO:
  template <class K, class V>
  class Map {
   public:
    struct Entry {
      K key;
      V value;

      bool isEmpty() const {
        return key == K();
      }
    };

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

      while (!entries_[index].isEmpty() && !(entries_[index].key == key)) {
        index = (index + 1) % capacity_;
      }

      entries_[index].key = key;
      entries_[index].value = value;
    }

    // TODO: Optimize
    void putAll(const Map<K, V>& other) {
      for (int i = 0; i < other.capacity(); ++i) {
        Entry* e = other.getEntry(i);
        if (!e->isEmpty()) put(e->key, e->value);
      }
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

    int size() const {
      return count_;
    }

    int capacity() const {
      return capacity_;
    }

    Entry* entries() const {
      return entries_;
    }

    Entry* getEntry(int index) const {
      return &entries_[index];
    }

   private:
    int findIndex(const K& key) const {
      if (capacity_ == 0) return -1;

      int index = static_cast<int>(key.hashCode() & 0x7fffffff) % capacity_;

      while (true) {
        if (entries_[index].key == key) return index;

        if (entries_[index].isEmpty()) return -1;

        index = (index + 1) % capacity_;
      }
    }

    void ensureCapacity() {
      if (count_ <= capacity_ * MAX_LOAD_PERCENT / 100) return;

      int oldCapacity = capacity_;
      int newCapacity = MIN_CAPACITY;
      if (oldCapacity >= MIN_CAPACITY) {
        newCapacity = oldCapacity * GROW_FACTOR;
      }

      Entry* oldEntries = entries_;

      void* mem = Memory::reallocate(nullptr, 0, sizeof(Entry) * newCapacity);
      entries_ = ::new (mem) Entry[newCapacity];
      // New capacity must be set 'after' new entries are allocated, otherwise null entries are
      // returned during the GC.
      capacity_ = newCapacity;

      if (oldEntries != nullptr) {
        for (int i = 0; i < oldCapacity; i++) {
          if (!oldEntries[i].isEmpty()) put(oldEntries[i].key, oldEntries[i].value);
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
