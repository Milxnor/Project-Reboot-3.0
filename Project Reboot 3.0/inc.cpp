#include "inc.h"

#include "Array.h"

/*

void* InstancedAllocator::Allocate(AllocatorType type, size_t Size)
{
	switch (type)
	{
	case AllocatorType::VIRTUALALLOC:
		return VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	case AllocatorType::FMEMORY:
		return FMemory::Realloc(0, Size, 0);
	}

	return nullptr;
} 

*/