#pragma once

#include "WeakObjectPtr.h"
#include "Object.h"
#include "PointerIsConvertibleFromTo.h"

template<class T = UObject, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr;

template<class T, class TWeakObjectPtrBase>
struct TWeakObjectPtr : public TWeakObjectPtrBase
{
	T* Get()
	{
		return (T*)TWeakObjectPtrBase::Get();
	}

	/*
	bool operator==(const TWeakObjectPtr& other)
	{
		return TWeakObjectPtrBase::operator==(other);
	}
	*/

	TWeakObjectPtr() {}
};

template <typename LhsT, typename RhsT, typename OtherTWeakObjectPtrBase>
FORCENOINLINE bool operator==(const TWeakObjectPtr<LhsT, OtherTWeakObjectPtrBase>& Lhs, const RhsT* Rhs)
{
	// It's also possible that these static_asserts may fail for valid conversions because
	// one or both of the types have only been forward-declared.
	static_assert(TPointerIsConvertibleFromTo<RhsT, UObject>::Value, "TWeakObjectPtr can only be compared with UObject types");
	static_assert(TPointerIsConvertibleFromTo<LhsT, RhsT>::Value || TPointerIsConvertibleFromTo<RhsT, LhsT>::Value, "Unable to compare TWeakObjectPtr with raw pointer - types are incompatible");

	// NOTE: this constructs a TWeakObjectPtrBase, which has some amount of overhead, so this may not be an efficient operation
	return (const OtherTWeakObjectPtrBase&)Lhs == OtherTWeakObjectPtrBase(Rhs);
}

template <typename LhsT, typename RhsT, typename OtherTWeakObjectPtrBase>
FORCENOINLINE bool operator==(const LhsT* Lhs, const TWeakObjectPtr<RhsT, OtherTWeakObjectPtrBase>& Rhs)
{
	// It's also possible that these static_asserts may fail for valid conversions because
	// one or both of the types have only been forward-declared.
	static_assert(TPointerIsConvertibleFromTo<LhsT, UObject>::Value, "TWeakObjectPtr can only be compared with UObject types");
	static_assert(TPointerIsConvertibleFromTo<LhsT, RhsT>::Value || TPointerIsConvertibleFromTo<RhsT, LhsT>::Value, "Unable to compare TWeakObjectPtr with raw pointer - types are incompatible");

	// NOTE: this constructs a TWeakObjectPtrBase, which has some amount of overhead, so this may not be an efficient operation
	return OtherTWeakObjectPtrBase(Lhs) == (const OtherTWeakObjectPtrBase&)Rhs;
}

template <typename LhsT, typename OtherTWeakObjectPtrBase>
FORCENOINLINE bool operator==(const TWeakObjectPtr<LhsT, OtherTWeakObjectPtrBase>& Lhs, TYPE_OF_NULLPTR)
{
	return !Lhs.IsValid();
}

template <typename RhsT, typename OtherTWeakObjectPtrBase>
FORCENOINLINE bool operator==(TYPE_OF_NULLPTR, const TWeakObjectPtr<RhsT, OtherTWeakObjectPtrBase>& Rhs)
{
	return !Rhs.IsValid();
}