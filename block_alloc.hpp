#ifndef BLOCK_ALLOC_HPP
#define BLOCK_ALLOC_HPP

namespace mempool{

typedef void (*destructor_t)(void*);

//默认的allocate
template <typename PoolType>
class block_alloc{
public:
    block_alloc(PoolType* p)
    : begin_(nullptr)
    , end_(nullptr)
    , destroy_chain_(nullptr)
    , pool_(p){
        init();
    }      
    ~block_alloc(){
        clear();
    }
    block_alloc(const block_alloc&) = delete;
    block_alloc& operator=(const block_alloc&) = delete;

private:
    constexpr static int BlockHeader = sizeof(void*);
    constexpr static int BlockSize = AllocSize -BlockHeader;
    /* 
     malloc分配内存大小是64k, 首先要减去16，这个是malloc中header结构体大小，其结构如下
     typedef long Align;  
     union header{     
     struct {  
         union header *ptr;    
          unsigned size;     
      }s;  
          Align x;   
     };  
     如果使用malloc(n)分配内存，malloc实际返回用户的内存是sizeof(header)+n+内存对齐填充的内存
     BlockHeader是prev指针大小
    */
    struct block{
      	block* prev;
		char buffer[BlockSize];
    };

    struct destory_node{
        destory_node* prev;
        destructor_t fn_destory;
    };

    //可用内存块起始地址
    char* begin_;
    //可用内存块结束地址
    char* end_;
    //用于有析构函数的类型，只析构不释放内存
    destory_node* destroy_chain_;
    //内存池
    PoolType* pool_;

public:
    inline void* allocate(size_t n){
		if ((size_t)(end_ - begin_) >= n)
		{
			return end_ -= n;
		}
		return do_allocate(n);
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
			end_ = (char*)node + pool_->alloc_size(node);
			return end_ -= n;
		}
	}
};

}



#endif /*BLOCK_ALLOC_HPP*/