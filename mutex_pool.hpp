#ifndef MUTEX_POOL_HPP
#define MUTEX_POOL_HPP
//线程安全的内存池，使用lock-free链表实现

#include <mutex>

namespace mempool{
template <class AllocT>
class mutex_pool{
public:
    mutex_pool(int free_limit = INT_MAX)
        : node_count_(1)
        , node_count_limit_(free_limit/chunk_size_){
        free_list_ = reinterpret_cast<chunk*>(AllocT::allocate(chunk_size_));
        free_list_->next = nullptr;
        alloc_size_ = AllocT::alloc_size(free_list_);
    }      
    ~mutex_pool(){
        clear();
    }
    mutex_pool(const mutex_pool&) = delete;
    mutex_pool& operator=(const mutex_pool&) = delete;

private:
    constexpr static int chunk_size_ = AllocSize;

    struct chunk{
        chunk* next;
    };

    //自由链表
    chunk* free_list_;
 
    //自由链表节点数
    int node_count_;
    //自由链表节点限制
    const int node_count_limit_ = 1;
    //系统实际分配内存
    size_t alloc_size_;
    std::mutex mutex_;

public:
    void* allocate(size_t n){
        if(n > static_cast<size_t>(chunk_size_)){
            return AllocT::allocate(n);
        }else{
            //双检查锁
            if(free_list_){
                std::lock_guard<std::mutex> guard(mutex_);
                if(free_list_){
                    chunk* node = free_list_;
                    free_list_ = node->next;
                    --node_count_;
                     return node; 
                }           
            } 
            return AllocT::allocate(chunk_size_);
        }
    }
    void  deallocate(void* p){
        if(AllocT::alloc_size(p) > alloc_size_ || node_count_ > node_count_limit_){
            AllocT::deallocate(p);
        }else{
            std::lock_guard<std::mutex> guard(mutex_);
			((chunk*)p)->next = free_list_;
			free_list_ = (chunk*)p;
			++node_count_;
        }
    }

    static size_t alloc_size(void* p){
        return AllocT::alloc_size(p);
    }

private:
   void clear(){
		while (free_list_)
		{
			chunk* node = free_list_;
			free_list_ = node->next;
			AllocT::deallocate(node);
		}
		node_count_ = 0;
   }
};
}




#endif /*MUTEX_POOL_HPP*/