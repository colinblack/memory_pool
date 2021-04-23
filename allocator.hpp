#ifndef ALLOCATOR_HPP_
#define ALLOCATOR_HPP_
#include "type_traits.hpp"

#define NEW_ARRAY(alloc, Type, count)   mempool::array_factory<Type>::create(alloc, count)



#endif /*ALLOCATOR_HPP_*/