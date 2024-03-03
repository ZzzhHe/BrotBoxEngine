#include "BBE/AllocBlock.h"
#include "BBE/List.h"
#include "BBE/Math.h"
#include "BBE/Logging.h"
#include <cstdlib>

static bool isBlockZero(void* ptr, size_t size)
{
	char* cptr = (char*)ptr;
	return cptr[0] == 0 && !memcmp(cptr, cptr + 1, size - 1);
}

static bbe::List<void*> storedBlocks[64];
static constexpr size_t PAGE_SIZE = 4096;

bbe::AllocBlock bbe::allocateBlock(size_t size)
{
	if (size == 0) return AllocBlock{ nullptr, 0 };

	auto access = bbe::Math::log2Floor(size);
	if (!bbe::Math::isPowerOfTwo(size))
	{
		size = (((size_t)1) << (access + 1));
		access++;
	}
	
	if (!storedBlocks[access].isEmpty())
	{
		void* addr = storedBlocks[access].popBack();
#ifdef _DEBUG
		if (!isBlockZero(addr, size))
		{
			throw IllegalStateException();
		}
#endif
		return AllocBlock{ addr, size };
	}

	if (size < PAGE_SIZE)
	{
		char* page = (char*)std::malloc(PAGE_SIZE);
		if (!page) throw NullPointerException();
		for (size_t i = 1; i < PAGE_SIZE / size; i++)
		{
			storedBlocks[access].add(page + i * size);
		}
		return AllocBlock{ page , size };
	}
	else
	{
		void* ptr = std::malloc(size);
		if (!ptr) throw NullPointerException();
		return AllocBlock{ ptr , size };
	}
}

void bbe::freeBlock(AllocBlock& block)
{
	if (block.data == nullptr || block.size == 0) return;

	auto access = bbe::Math::log2Floor(block.size);
	storedBlocks[access].add(block.data);
#ifdef _DEBUG
	memset(block.data, 0, block.size);
#endif

	block.data = nullptr;
	block.size = 0;
}



// From: https://en.cppreference.com/w/cpp/memory/new/operator_new
// See for license: https://en.cppreference.com/w/Cppreference:FAQ
// Quote:
// What can I do with the material on this site?
// The content is licensed under Creative Commons Attribution - Sharealike 3.0 
// Unported License(CC - BY - SA) and by the GNU Free Documentation License(GFDL)
// (unversioned, with no invariant sections, front - cover texts, or back - cover
// texts).That means that you can use this site in almost any way you like,
// including mirroring, copying, translating, etc.All we would ask is to provide
// link back to cppreference.com so that people know where to get the most up -
// to - date content.In addition to that, any modified content should be released
// under an equivalent license so that everyone could benefit from the modified
// versions.

//#define CHECK_ALLOCS
#ifdef CHECK_ALLOCS
#include <cstdio>
#include <cstdlib>
#include <new>
 
// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
    std::printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
 
// no inline, required by [replacement.functions]/3
void* operator new[](std::size_t sz)
{
    std::printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success
 
    if (void *ptr = std::malloc(sz))
        return ptr;
 
    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
 
void operator delete(void* ptr) noexcept
{
    std::puts("3) delete(void*)");
    std::free(ptr);
}
 
void operator delete(void* ptr, std::size_t size) noexcept
{
    std::printf("4) delete(void*, size_t), size = %zu\n", size);
    std::free(ptr);
}
 
void operator delete[](void* ptr) noexcept
{
    std::puts("5) delete[](void* ptr)");
    std::free(ptr);
}
 
void operator delete[](void* ptr, std::size_t size) noexcept
{
    std::printf("6) delete[](void*, size_t), size = %zu\n", size);
    std::free(ptr);
}
#endif
