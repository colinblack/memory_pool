#ifndef SYS_ALLOC_HPP
#define SYS_ALLOC_HPP


#define SYS_MALLOC_SIZE 65536
#define SYS_MALLOC_HEADER 16
#define ALIGNAS_SIZE 8 //对齐字节必须是8倍数
#define ALIGNAS_BITS 3 //2^3

namespace mempool{
    constexpr static int MallocSize = SYS_MALLOC_SIZE;/*64k*/
    constexpr static int MallocHeader = SYS_MALLOC_HEADER;
    constexpr static int AllocSize =MallocSize- MallocHeader;

class sys_alloc{
public:
    static void* allocate(size_t n){
        void* p = nullptr;
        if(posix_memalign(&p, alignof(size_t), 
                          sizeof(size_t) * n) != 0){
             throw std::bad_alloc();
        }

        //return malloc(n);
        return p;        
    };
    static void deallocate(void* p){
        free(p);
    };
    static size_t alloc_size(void* p){
        return malloc_usable_size(p);
    }
};
}




#endif /*SYS_ALLOC_HPP*/