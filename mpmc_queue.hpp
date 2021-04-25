#ifndef MPMC_QUEUE_HPP
#define MPMC_QUEUE_HPP

#include <tuple>

namespace mempool{
namespace mpmc{
static constexpr size_t hardwareInterferenceSize = 8;

struct slot{
    void* node = nullptr;
    alignas(hardwareInterferenceSize) std::atomic<size_t> turn = {0};
};

#define QUEUE_CAPACITY 64   //capacity必须是2^n
#define QUEUE_BITS     6    //2^n中的n

class queue{
public:
    explicit queue()
        : slots_(nullptr), head_(0), tail_(0){
     //     std::cout << capacity_ << "  " << bits_ << std::endl;
        if (capacity_ < 1) {
            throw std::invalid_argument("capacity < 1");
        }

        if(posix_memalign(reinterpret_cast<void **>(&slots_), alignof(slot), 
                          sizeof(slot) * (capacity_+1)) != 0){
             throw std::bad_alloc();
        }
        
        if(reinterpret_cast<size_t>(slots_) % alignof(slot) != 0){
            free(slots_);
            throw std::bad_alloc();
        }
    }

    ~queue(){
        for(size_t i = 0; i < capacity_; ++i){
          free(slots_[i].node);
        }

        free(slots_);    
    }

    queue(const queue &) = delete;
    queue &operator=(const queue &) = delete;

    void emplace(void* p) noexcept {
        auto const head = head_.fetch_add(1);
        auto &slot = slots_[idx(head)];
        while (turn(head) << 1 != slot.turn.load(std::memory_order_acquire))
            ;
        slot.node = p;
        slot.turn.store((turn(head) << 1) + 1, std::memory_order_release);
    }


    bool try_emplace(void* p) noexcept {
        auto head = head_.load(std::memory_order_acquire);
        for (;;) {
            auto &slot = slots_[idx(head)];
            if (turn(head) << 1 == slot.turn.load(std::memory_order_acquire)) {
            if (head_.compare_exchange_strong(head, head + 1)) {
                slot.node = p;
                slot.turn.store((turn(head) << 1) + 1, std::memory_order_release);
                return true;
            }
        } else {
            auto const prev_head = head;
            head = head_.load(std::memory_order_acquire);
            if (head == prev_head) {
                return false;
            }
            }
        }
    }

    void push(void* p) noexcept {
        emplace(p);
    }    

    bool try_push(void* p) noexcept {
        return try_emplace(p);
    }


  void pop(void*& p) noexcept {
    auto const tail = tail_.fetch_add(1);
    auto &slot = slots_[idx(tail)];
    while ((turn(tail) << 1) + 1 != slot.turn.load(std::memory_order_acquire))
        ;
    p = slot.node;
    slot.node = nullptr;
    slot.turn.store((turn(tail) << 1) + 2, std::memory_order_release);
  }

  bool try_pop(void*& p) noexcept {
    auto tail = tail_.load(std::memory_order_acquire);
    for (;;) {
      auto &slot = slots_[idx(tail)];
      if ((turn(tail) << 1) + 1 == slot.turn.load(std::memory_order_acquire)) {
        if (tail_.compare_exchange_strong(tail, tail + 1)) {
          p = slot.node;
          slot.node = nullptr;
          slot.turn.store((turn(tail) << 1) + 2, std::memory_order_release);
          return true;
        }
      } else {
        auto const prev_tail = tail;
        tail = tail_.load(std::memory_order_acquire);
        if (tail == prev_tail) {
          return false;
        }
      }
    }
  }

private:
  //constexpr size_t idx(size_t i) const noexcept { return i % capacity_; }
  constexpr size_t idx(size_t i) const noexcept { return i &(capacity_-1); }
  //constexpr size_t turn(size_t i) const noexcept { return i / capacity_; }
  constexpr size_t turn(size_t i) const noexcept { return i >> bits_; }

private:
    constexpr static size_t capacity_ = QUEUE_CAPACITY;
    constexpr static size_t bits_ = QUEUE_BITS;
    slot* slots_;
    alignas(hardwareInterferenceSize) std::atomic<size_t> head_;
    alignas(hardwareInterferenceSize) std::atomic<size_t> tail_;
};

}
}

#endif /*MPMC_QUEUE_HPP */
