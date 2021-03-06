#ifndef ALLOCATOR_HPP_
#define ALLOCATOR_HPP_

#include <cstdlib>
#include <malloc.h>
#include <atomic>
#include <climits>
#include <atomic>

#include "sys_alloc.hpp"
#include "mpmc_pool.hpp"
#include "chunk_pool.hpp"
#include "mpmc_pool.hpp"
#include "cycle_alloc.hpp"
#include "block_alloc.hpp"
#include "type_traits.hpp"
#include "mutex_pool.hpp"

#define NEW_ARRAY(alloc, Type, count)   mempool::array_factory<Type>::create(alloc, count)
#define RECYCLE_ARRAY(alloc, Type, point, size)    mempool::array_factory<Type>::recycle(alloc, point, size)

typedef mempool::chunk_pool<mempool::sys_alloc> ChunkPool;
typedef mempool::mpmc_pool<mempool::sys_alloc> MpmcPool;
typedef mempool::mutex_pool<mempool::sys_alloc>  MutexPool;

#define  DefaultPool MutexPool
typedef mempool::block_alloc<DefaultPool> DefaultAlloc;

typedef mempool::block_alloc<MpmcPool> MpmcAlloc;
typedef mempool::block_alloc<MutexPool> MutexAlloc;

typedef mempool::cycle_alloc<MpmcPool> MpmcCycleAlloc;

#endif /*ALLOCATOR_HPP_*/