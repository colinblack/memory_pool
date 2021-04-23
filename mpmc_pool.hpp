#ifndef MPMC_POOL_HPP
#define MPMC_POOL_HPP
//线程安全的内存池，使用lock-free链表实现

#include "mpmc_queue.hpp"

namespace mempool{

const size_t PageSize=4096;

template <class AllocT>
class mpmc_pool{
public:
    mpmc_pool(int free_limit = INT_MAX)
        : free_list_(round_up(free_limit/chunk_size_)){
        Chunk node = reinterpret_cast<Chunk>(AllocT::allocate(chunk_size_));
        alloc_size_ = AllocT::alloc_size(node);
        free_list_.push(std::move(node));
    }      
    ~mpmc_pool(){}
    mpmc_pool(const mpmc_pool&) = delete;
    mpmc_pool& operator=(const mpmc_pool&) = delete;

private:
    constexpr static int chunk_size_ = AllocSize;

    //自由链表
    typedef void* Chunk;
    typedef mpmc_queue<Chunk> FreeList;
    FreeList free_list_;
    //系统实际分配内存
    size_t alloc_size_;

public:
    void* allocate(size_t n){
        if(n > static_cast<size_t>(chunk_size_)){
            return AllocT::allocate(n);
        }else{
            Chunk node = nullptr;
            if(!free_list_.try_pop(node)){
                return AllocT::allocate(chunk_size_); 
            }
            return node;
        }
    }
    void  deallocate(void* p){
        if(AllocT::alloc_size(p) > alloc_size_){
            AllocT::deallocate(p);
        }else{
           if(!free_list_.try_push(std::move(p))){
               AllocT::deallocate(p);
           }
        }

    }

    static size_t alloc_size(void* p){
        return AllocT::alloc_size(p);
    }

    static constexpr size_t round_up(size_t bytes) {
        return (((bytes) + PageSize-1) & ~(PageSize - 1));
    }
};

}




#endif /*MPMC_POOL_HPP*/