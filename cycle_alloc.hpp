#ifndef CYCLE_ALLOC_HPP
#define CYCLE_ALLOC_HPP

namespace mempool{

typedef void (*destructor_t)(void*);

//默认的allocate
template <typename PoolType>
class cycle_alloc{
public:
    cycle_alloc(PoolType* p)
    : destroy_chain_(nullptr)
    , pool_(p){
        init();
    }      
    ~cycle_alloc(){
        clear();
    }
    cycle_alloc(const cycle_alloc&) = delete;
    cycle_alloc& operator=(const cycle_alloc&) = delete;

private:
    constexpr static int BlockHeader = sizeof(void*);
    constexpr static int BlockSize = AllocSize -BlockHeader;
    struct block{
      	block* prev;
		char buffer[BlockSize];
    };

    struct chunk{
        chunk* next=nullptr;
    };

    chunk* free_list_[SYS_MALLOC_SIZE/ALIGNAS_SIZE]={nullptr};


    struct destory_node{
        destory_node* prev;
        destructor_t fn_destory;
    };

    char* begin_;
    char* end_;

    destory_node* destroy_chain_;
    PoolType* pool_;

public:
    void* allocate(size_t n){
		size_t bytes = round_up(n);
		//先看自由链表里有没有空闲内存
		chunk** l = free_list_ + freelist_index(bytes);
		if(*l){
			chunk* block = *l;
			*l = block->next;
			return block;
		}

		if ((size_t)(end_ - begin_) >= bytes)
		{
			return end_ -= bytes;
		}
		return do_allocate(bytes);
    }

    void recycle(void*p, size_t size){
        size_t index = freelist_index(size);
        chunk* node = reinterpret_cast<chunk*>(p);
        node->next = free_list_[index];
        free_list_[index] = node;  
    }    
    

private: 
    inline constexpr block* chain_header(const char* begin) {
		return (block*)(begin - BlockHeader);
	}

    inline void init(){
		begin_ = end_ = (char*)BlockHeader;
	}

    void clear(){
		while (destroy_chain_)
		{
			destory_node* curr = destroy_chain_;
			destroy_chain_ = destroy_chain_->prev;
			curr->fn_destory(curr + 1);
		}
		block* head = chain_header(begin_);
		while (head)
		{
			block* curr = head;
			head = head->prev;
			pool_->deallocate(curr);
		}
		begin_ = end_ = (char*)BlockHeader;
    }

    void* do_allocate(size_t n)
	{
		if (n >= BlockSize)
		{
			block* pHeader = chain_header(begin_);
			block* node = (block*)pool_->allocate(BlockHeader + n);
			if (pHeader)
			{
				node->prev = pHeader->prev;
				pHeader->prev = node;
			}
			else
			{
				end_ = begin_ = node->buffer;
				node->prev = NULL;
			}
			return node->buffer;
		}
		else
		{
			block* node = (block*)pool_->allocate(sizeof(block));
			node->prev = chain_header(begin_);
			begin_ = node->buffer;
			//end_ = (char*)node + pool_->alloc_size(node); 很可能出现内存地址不是8的倍数
			end_ = (char*)node + SYS_MALLOC_SIZE;
			return end_ -= n;
		}
	}

    constexpr static size_t freelist_index(size_t bytes) {
        return (((bytes) + ALIGNAS_SIZE-1)>>ALIGNAS_BITS - 1);
    }

	constexpr static size_t round_up(size_t bytes){
		return (bytes + ALIGNAS_SIZE-1) & ~(ALIGNAS_SIZE-1);
	}
};

}



#endif /*CYCLE_ALLOC_HPP*/