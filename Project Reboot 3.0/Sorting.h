#pragma once

#include "Sort.h"

template<typename T, class PREDICATE_CLASS>
struct TDereferenceWrapper
{
	const PREDICATE_CLASS& Predicate;

	TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
		: Predicate(InPredicate) {}

	/** Pass through for non-pointer types */
	FORCEINLINE bool operator()(T& A, T& B) { return Predicate(A, B); }
	FORCEINLINE bool operator()(const T& A, const T& B) const { return Predicate(A, B); }
};
/** Partially specialized version of the above class */
template<typename T, class PREDICATE_CLASS>
struct TDereferenceWrapper<T*, PREDICATE_CLASS>
{
	const PREDICATE_CLASS& Predicate;

	TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
		: Predicate(InPredicate) {}

	/** Dereference pointers */
	FORCEINLINE bool operator()(T* A, T* B) const
	{
		return Predicate(*A, *B);
	}
};

template <typename T>
struct TArrayRange
{
	TArrayRange(T* InPtr, int32 InSize)
		: Begin(InPtr)
		, Size(InSize)
	{
	}

	T* GetData() const { return Begin; }
	int32 Num() const { return Size; }

private:
	T* Begin;
	int32 Size;
};

template<class T, class PREDICATE_CLASS>
void Sort(T** First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
	TArrayRange<T*> ArrayRange(First, Num);
	Algo::Sort(ArrayRange, TDereferenceWrapper<T*, PREDICATE_CLASS>(Predicate));
}