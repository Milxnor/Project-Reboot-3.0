#pragma once

#include "inc.h"

#include "EnableIf.h"
#include "RemoveReference.h"
#include "AndOrNot.h"
#include "IsArithmetic.h"
#include "IsPointer.h"
#include "TypeCompatibleBytes.h"

template <typename T> struct TRValueToLValueReference { typedef T  Type; };
template <typename T> struct TRValueToLValueReference<T&&> { typedef T& Type; };

template<typename ReferencedType>
FORCEINLINE ReferencedType* IfAThenAElseB(ReferencedType* A, ReferencedType* B)
{
	using PTRINT = int64;
	const PTRINT IntA = reinterpret_cast<PTRINT>(A);
	const PTRINT IntB = reinterpret_cast<PTRINT>(B);

	// Compute a mask which has all bits set if IntA is zero, and no bits set if it's non-zero.
	const PTRINT MaskB = -(!IntA);

	return reinterpret_cast<ReferencedType*>(IntA | (MaskB & IntB));
}

template<typename T>//, typename = typename TEnableIf<TIsContiguousContainer<T>::Value>::Type>
auto GetData(T&& Container) -> decltype(Container.GetData())
{
	return Container.GetData();
}

template <typename T, SIZE_T N>
constexpr T* GetData(T(&Container)[N])
{
	return Container;
}

template <typename T>
constexpr T* GetData(std::initializer_list<T> List)
{
	return List.begin();
}

template<typename T>//, typename = typename TEnableIf<TIsContiguousContainer<T>::Value>::Type>
SIZE_T GetNum(T&& Container)
{
	return (SIZE_T)Container.Num();
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type& Obj)
{
	return (T&&)Obj;
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type&& Obj)
{
	return (T&&)Obj;
}

template <typename T>
FORCEINLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
	typedef typename TRemoveReference<T>::Type CastType;

	// Validate that we're not being passed an rvalue or a const object - the former is redundant, the latter is almost certainly a mistake
	// static_assert(TIsLValueReferenceType<T>::Value, "MoveTemp called on an rvalue");
	// static_assert(!TAreTypesEqual<CastType&, const CastType&>::Value, "MoveTemp called on a const object");

	return (CastType&&)Obj;
}

template <typename T>
struct TUseBitwiseSwap
{
	// We don't use bitwise swapping for 'register' types because this will force them into memory and be slower.
	enum { Value = !TOrValue<__is_enum(T), TIsPointer<T>, TIsArithmetic<T>>::Value };
};

template <typename T>
inline typename TEnableIf<!TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
{
	T Temp = MoveTemp(A);
	A = MoveTemp(B);
	B = MoveTemp(Temp);
}

#define LIKELY(x)			(x)

template <typename T>
inline typename TEnableIf<TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
{
	if (LIKELY(&A != &B))
	{
		TTypeCompatibleBytes<T> Temp;
		// FMemory::Memcpy(&Temp, &A, sizeof(T));
		// FMemory::Memcpy(&A, &B, sizeof(T));
		// FMemory::Memcpy(&B, &Temp, sizeof(T));
		memcpy(&Temp, &A, sizeof(T));
		memcpy(&A, &B, sizeof(T));
		memcpy(&B, &Temp, sizeof(T));
	}
}