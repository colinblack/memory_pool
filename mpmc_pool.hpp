#ifndef MPMC_POOL_HPP
#define MPMC_POOL_HPP
//线程安全的内存池，使用lock-free链表实现

#include "mpmc_queue.hpp"

namespace mempool{

#define PAGE_SIZE 4096
#define ALIGN_ARRAY_SIZE 17

constexpr int align_array[ALIGN_ARRAY_SIZE] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 
                                                          1024, 2048, 4096, 8192, 16384, 32768, 65536};

template <class AllocT>
class mpmc_pool{ 
public:
    mpmc_pool(int free_limit = INT_MAX)
        : free_list_(){
 //         : free_list_(round_up(4096)){
        Chunk node = AllocT::allocate(chunk_size_);
        alloc_size_ = AllocT::alloc_size(node);
        free_list_.push(node);
    }      
    ~mpmc_pool(){}
    mpmc_pool(const mpmc_pool&) = delete;
    mpmc_pool& operator=(const mpmc_pool&) = delete;

private:
    constexpr static int chunk_size_ = AllocSize;

    //自由链表
    typedef void* Chunk;
    typedef mpmc::queue FreeList;
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
           if(!free_list_.try_push(p)){
               AllocT::deallocate(p);
           }
        }

    }

    static size_t alloc_size(void* p){
        return AllocT::alloc_size(p);
    }

    constexpr static std::tuple<const size_t, const size_t> round_up(size_t bytes) {
     //   return (((bytes) + PAGE_SIZE-1) & ~(PAGE_SIZE - 1));
        //判断是否位2的n次方
/*         if((number & number-1) == 0){
            return number;
        }

        if(number >= (SIZE_MAX/2048)){
            return SIZE_MAX/2048+1;
        }

        number |= number >> 1;
        number |= number >> 2;
        number |= number >> 4;
        number |= number >> 8;
        number |= number >> 16;

        return number; */

        bytes /= PAGE_SIZE;      
        int l = 0;
        int r = ALIGN_ARRAY_SIZE - 1;

        while (l <= r)  
        {
            int m = l + ((r - l) >> 1);
        
            if (static_cast<size_t>(align_array[m]) >= bytes)
                r = m - 1;
            else
                l = m + 1;
        }
        
        size_t size =  (l < ALIGN_ARRAY_SIZE) ? align_array[l] : align_array[ALIGN_ARRAY_SIZE-1];
        size_t index = (l < ALIGN_ARRAY_SIZE) ? l : ALIGN_ARRAY_SIZE-1;
    
        return std::make_tuple(size, index);    
    }
};

}




#endif /*MPMC_POOL_HPP*/