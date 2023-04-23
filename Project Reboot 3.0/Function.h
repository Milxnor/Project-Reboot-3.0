// FROM 4.23

#pragma once

#include "TypeCompatibleBytes.h"

#if defined(_WIN32) && !defined(_WIN64)
// Don't use inline storage on Win32, because that will affect the alignment of TFunction, and we can't pass extra-aligned types by value on Win32.
#define TFUNCTION_USES_INLINE_STORAGE 0
#elif USE_SMALL_TFUNCTIONS
#define TFUNCTION_USES_INLINE_STORAGE 0
#else
#define TFUNCTION_USES_INLINE_STORAGE 1
#define TFUNCTION_INLINE_SIZE         32
#define TFUNCTION_INLINE_ALIGNMENT    16
#endif

template <typename StorageType, typename FuncType>
struct TFunctionRefBase;

template <typename StorageType, typename Ret, typename... ParamTypes>
struct TFunctionRefBase<StorageType, Ret(ParamTypes...)>
{
	Ret(*Callable)(void*, ParamTypes&...);

	StorageType Storage;

#if ENABLE_TFUNCTIONREF_VISUALIZATION
	// To help debug visualizers
	TDebugHelper<void> DebugPtrStorage;
#endif
};

struct FFunctionStorage
{
	FFunctionStorage()
		: HeapAllocation(nullptr)
	{
	}

	void* HeapAllocation;
#if TFUNCTION_USES_INLINE_STORAGE
	// Inline storage for an owned object
	TAlignedBytes<TFUNCTION_INLINE_SIZE, TFUNCTION_INLINE_ALIGNMENT> InlineAllocation;
#endif
};

template <bool bUnique>
struct TFunctionStorage : FFunctionStorage
{
};

template <typename FuncType>
class TFunction final : public TFunctionRefBase<TFunctionStorage<false>, FuncType>
{
};