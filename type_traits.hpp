#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

#include <cstdlib>
#include "block_alloc.hpp"
//支持没有构造函数的C类型

//有构造和析构的类型使用placement new和显式析构
namespace mempool{
template <class Type>
struct constructor_traits
{
	static void  construct(void* data)
	{
		new(data) Type;
	}

	static void  constructArray(Type* array, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			new(array + i) Type;
	}
};


//特化没有构造和析构的C类型
#define TYPE_NO_CONSTRUCTOR(Type) \
    template <>															\
    struct constructor_traits<Type>									    \
    {																	\
        static inline void  construct(void* data) {}					\
        static inline void  constructArray(Type*, size_t) {}			\
    }; 



template <class Type>
struct destructor_traits
{
	enum { has_destructor = 1 };

	static void destruct(void* data)
	{
		((Type*)data)->~Type();
	}

	static void destructArray(Type* array, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
			array[i].~Type();
	}
};


#define TYPE_NO_DESTRUCTOR(Type) \
    template <>																	\
    struct destructor_traits<Type>										        \
    {																			\
        typedef int destructor_type;											\
                                                                                \
        enum { has_destructor = 0 };												\
        enum { destruct = 0 };													\
                                                                                \
        static inline void destructArray(Type*, size_t) {}			            \
    };



#define DECLE_C_TYPE(Type) \
    TYPE_NO_CONSTRUCTOR(Type);\
    TYPE_NO_DESTRUCTOR(Type)


DECLE_C_TYPE(bool);
DECLE_C_TYPE(float);
DECLE_C_TYPE(double);

DECLE_C_TYPE(int);
DECLE_C_TYPE(unsigned int);

DECLE_C_TYPE(char);
DECLE_C_TYPE(unsigned char);

DECLE_C_TYPE(short);
DECLE_C_TYPE(unsigned short);

DECLE_C_TYPE(long);
DECLE_C_TYPE(unsigned long);



template <class Type>
struct array_factory_no_destructor
{
	enum { destruct = 0 };

	typedef void destructor_header;

	static size_t alloc_size(size_t count)
	{
		return sizeof(Type)*count;
	}

	template <class AllocT>
	static Type* create(AllocT& alloc, size_t count)
	{
		Type* array = (Type*)alloc.allocate(sizeof(Type)*count);
		constructor_traits<Type>::constructArray(array, count);
		return array;
	}

	static char* buffer(Type* array)
	{
		return (char*)array;
	}
};

template <class Type, int hasDestructor = 0>
struct array_factory : array_factory_no_destructor<Type> {};


}


#endif /*TYPE_TRAITS_HPP*/




