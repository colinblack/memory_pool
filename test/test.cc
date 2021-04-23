#include <iostream>
#include <cstddef>
#include <vector>
#include <thread>
#include <time.h>
#include "allocator.hpp"

void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;

	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&, k]() {
			void** v = (void**)malloc(sizeof(void*)*ntimes);

			for (size_t j = 0; j < rounds; ++j)
			{
				int size = (rand() % 4096);
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v[i] = malloc(size);
				}
				size_t end1 = clock();

				for (size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}

				malloc_costtime += end1 - begin1;
			}
			free(v);
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	printf("%lu个线程并发执行%lu轮次，每轮次malloc %lu次: 花费：%lu ms\n",
		nworks, rounds, ntimes, malloc_costtime);

}



void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;
	mempool::default_pool recycle;


	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			mempool::default_alloc alloc(&recycle);
			void** v = (void**)malloc(sizeof(void*)*ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				int size = (rand() % 256);
			//	printf("size=%d\n", size);
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v[i] = NEW_ARRAY(alloc, char, size);
				}
				size_t end1 = clock();


				malloc_costtime += end1 - begin1;
			}
			free(v);
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	printf("%lu个线程并发执行%lu轮次，每轮次concurrent malloc %lu次: 花费：%lu ms\n",
		nworks, rounds, ntimes, malloc_costtime);
}



int main(){
    srand((unsigned)time(NULL));
    printf("==============================================================================\n");
    BenchmarkMalloc(100, 16, 10);
    printf("==============================================================================\n");
    BenchmarkConcurrentMalloc(100, 16, 10);
    return 0;
}
