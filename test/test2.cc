#include <iostream>
#include <cstddef>
#include <vector>
#include <thread>
#include <time.h>
#include "unistd.h"
#include "allocator.hpp"

void BenchmarkMpmCyclecMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;
	MpmcPool recycle;

	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			MpmcCycleAlloc alloc(&recycle);
			void** v = (void**)malloc(sizeof(void*)*ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				int size = ((rand()+1) % 256);
				//printf("size=%d\n", size);
		       		size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v[i] = NEW_ARRAY(alloc, char, size);
				}
				size_t end1 = clock();
				malloc_costtime += end1 - begin1;
#if 0
				for (size_t i = 0; i < ntimes; i++)
				{
					RECYCLE_ARRAY(alloc, char, v[i], size);
				}
#endif
			}
			free(v);
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	printf("%lu个线程并发执行%lu轮次，每轮次无锁cycle malloc %lu次: 花费：%lu ms\n",
		nworks, rounds, ntimes, malloc_costtime);
}



int main(){
    srand((unsigned)time(NULL));
    BenchmarkMpmCyclecMalloc(10000, 16, 30);
    printf("==============================================================================\n");
    while(1);

    return 0;
}
