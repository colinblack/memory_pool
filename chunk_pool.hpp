#ifndef CHUNK_POOL_HPP
#define CHUNK_POOL_HPP
//线程安全的内存池，使用lock-free链表实现

#include <atomic>
#include <climits>
#include "sys_alloc.hpp"


namespace mempool{
template <class AllocT>
class chunk_pool{
public:
    chunk_pool(int free_limit = INT_MAX)
        : node_count_(1)
        , node_count_limit_(free_limit/chunk_size_){
        head_ = tail_ = reinterpret_cast<chunk*>(AllocT::allocate(chunk_size_));
        chunk* head = head_.load();
        chunk* tail = tail_.load();
        head->next = tail->next = nullptr;
        alloc_size_ = AllocT::alloc_size(head);
    }      
    ~chunk_pool(){
        clear();
    }
    chunk_pool(const chunk_pool&) = delete;
    chunk_pool& operator=(const chunk_pool&) = delete;

private:
    constexpr static int chunk_size_ = AllocSize;

    struct chunk{
        std::atomic<chunk*> next;
    };

    typedef std::atomic<chunk*> FreeList;
    //自由链表
    FreeList head_;
    FreeList tail_;
   
    //自由链表节点数
    std::atomic<int> node_count_;
    //自由链表节点限制
    const int node_count_limit_ = 1;
    //系统实际分配内存
    size_t alloc_size_;


public:
    void* allocate(size_t n){
        if(n > static_cast<size_t>(chunk_size_)){
            return AllocT::allocate(n);
        }else{
            if(node_count_.load() > 0){
                chunk* head = head_.load();
                chunk* tail = tail_.load();
                while(true){
                    //取出头指针，尾指针，和第一个元素的指针
                    chunk* next = head->next;
                     // Q->head 指针已移动，重新取 head指针
                    if(head != head_.load())
                        continue;
                     // 如果是空队列
                    if ( head == tail && next == nullptr ) {
                        return AllocT::allocate(chunk_size_);
                    }
                    //如果 tail 指针落后了
                    if ( head == tail && next == nullptr ) {
                        atomic_compare_exchange_strong(&tail_, &tail, next);
                        continue;
                    }
                    //移动 head 指针成功后，取出数据
                    if (atomic_compare_exchange_weak(&head_, &head, next)){
                        break;
                    }
                }
                node_count_--;
                return head;
            }
            return AllocT::allocate(chunk_size_);
        }
    }
    void  deallocate(void* p){
        if(AllocT::alloc_size(p) > alloc_size_ || node_count_.load() > node_count_limit_){
            AllocT::deallocate(p);
        }else{
            chunk* node = reinterpret_cast<chunk*>(p);
            node->next = nullptr;
            chunk* tail = tail_.load();
            chunk* next = tail->next;
            while(true) {
                //先取一下尾指针和尾指针的next
                //如果尾指针已经被移动了，则重新开始
                if (tail != tail_.load()) 
                    continue;
                //如果尾指针的 next 不为NULL，则 fetch 全局尾指针到next
                if (next != nullptr) {
                    atomic_compare_exchange_weak(&tail_, &tail, next);
                    continue;
                }
                //如果加入结点成功，则退出
                if (atomic_compare_exchange_weak(&tail->next, &next, node))
                   break;
            }
            atomic_compare_exchange_strong(&tail_, &tail, node); //置尾结点
            node_count_++;
        }
    }

    static size_t alloc_size(void* p){
        return AllocT::alloc_size(p);
    }

private:
   void clear(){
        chunk* head = head_.load();
       	while (head)
		{
			chunk* cur = head;
			head = cur->next;
			AllocT::deallocate(cur);
		}
		node_count_.store(0);
   }
};

typedef chunk_pool<sys_alloc> default_pool;

}




#endif /*CHUNK_POOL_HPP*/