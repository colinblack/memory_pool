#include <iostream>
#include <cstddef>
#include <vector>
#include <thread>
#include <time.h>
#include "unistd.h"
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
			//	int size = (rand() % 4096);
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v[i] = malloc(16);
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



void BenchmarkMutexMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;
	MutexPool recycle;


	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			MutexAlloc alloc(&recycle);
			void** v = (void**)malloc(sizeof(void*)*ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				//int size = (rand() % 256);
			//	printf("size=%d\n", size);
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v[i] = NEW_ARRAY(alloc, char, 16);
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

	printf("%lu个线程并发执行%lu轮次，每轮次加锁 malloc %lu次: 花费：%lu ms\n",
		nworks, rounds, ntimes, malloc_costtime);
}

void BenchmarkMpmcMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;
	MpmcPool recycle;


	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			MpmcAlloc alloc(&recycle);
			void** v = (void**)malloc(sizeof(void*)*ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				//int size = (rand() % 256);
			//	printf("size=%d\n", size);
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v[i] = NEW_ARRAY(alloc, char, 16);
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

	printf("%lu个线程并发执行%lu轮次，每轮次无锁 malloc %lu次: 花费：%lu ms\n",
		nworks, rounds, ntimes, malloc_costtime);
}



int main(){
//    srand((unsigned)time(NULL));
//    printf("==============================================================================\n");
//    BenchmarkMalloc(10000, 16, 30);
    printf("==============================================================================\n");
//	sleep(20);
//    BenchmarkMpmcMalloc(10000, 16, 30);
//	printf("==============================================================================\n");
//	sleep(20);
	BenchmarkMutexMalloc(10000, 16, 30);

    while(1);
    return 0;
}
