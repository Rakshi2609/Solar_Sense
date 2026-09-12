// Fixed-size RAM ring of records waiting to reach the backend.
//
// Nothing here is dynamic: the buffer is sized at compile time so an outage
// cannot fragment the heap on a device meant to run for weeks. When it fills,
// the oldest record is dropped and counted -- a silent drop would look like
// clean data with a hole in it.
#pragma once

#include <Arduino.h>

#include "../data/SensorData.h"

template <size_t N>
class RecordBuffer {
 public:
  size_t size() const { return count_; }
  size_t capacity() const { return N; }
  bool empty() const { return count_ == 0; }
  uint32_t dropped() const { return dropped_; }

  void push(const SensorData& d) {
    items_[head_] = d;
    head_ = (head_ + 1) % N;
    if (count_ == N) {
      tail_ = (tail_ + 1) % N;  // overwrote the oldest
      dropped_++;
    } else {
      count_++;
    }
  }

  // Oldest first: a backlog should reach the database in the order it happened.
  bool peek(SensorData& out) const {
    if (count_ == 0) return false;
    out = items_[tail_];
    return true;
  }

  void pop() {
    if (count_ == 0) return;
    tail_ = (tail_ + 1) % N;
    count_--;
  }

 private:
  SensorData items_[N];
  size_t head_ = 0;
  size_t tail_ = 0;
  size_t count_ = 0;
  uint32_t dropped_ = 0;
};
