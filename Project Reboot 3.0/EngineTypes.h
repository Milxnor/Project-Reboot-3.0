#pragma once

#include "Object.h"
#include "Vector.h"

#include "DelegateCombinations.h"

enum class ESpawnActorCollisionHandlingMethod : uint8
{
	Undefined,
	AlwaysSpawn,
	AdjustIfPossibleButAlwaysSpawn,
	AdjustIfPossibleButDontSpawnIfColliding,
	DontSpawnIfColliding
};

struct FHitResult
{
	static class UStruct* GetStruct();
	static int GetStructSize();

	bool IsBlockingHit();
	FVector& GetLocation();
	void CopyFromHitResult(FHitResult* Other);
};

struct FTimerHandle
{
	FTimerHandle()
		: Handle(0)
	{
	}

	/** True if this handle was ever initialized by the timer manager */
	bool IsValid() const
	{
		return Handle != 0;
	}

	/** Explicitly clear handle */
	void Invalidate()
	{
		Handle = 0;
	}

	bool operator==(const FTimerHandle& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FTimerHandle& Other) const
	{
		return Handle != Other.Handle;
	}

	/* FString ToString() const
	{
		return FString::Printf(TEXT("%llu"), Handle);
	} */

// private:
	static const uint32 IndexBits = 24;
	static const uint32 SerialNumberBits = 40;

	static_assert(IndexBits + SerialNumberBits == 64, "The space for the timer index and serial number should total 64 bits");

	static const int32  MaxIndex = (int32)1 << IndexBits;
	static const uint64 MaxSerialNumber = (uint64)1 << SerialNumberBits;

	void SetIndexAndSerialNumber(int32 Index, uint64 SerialNumber)
	{
		// check(Index >= 0 && Index < MaxIndex);
		// check(SerialNumber < MaxSerialNumber);
		Handle = (SerialNumber << IndexBits) | (uint64)(uint32)Index;
	}

	FORCEINLINE int32 GetIndex() const
	{
		return (int32)(Handle & (uint64)(MaxIndex - 1));
	}

	FORCEINLINE uint64 GetSerialNumber() const
	{
		return Handle >> IndexBits;
	}

	uint64 Handle;

	/* friend uint32 GetTypeHash(const FTimerHandle& InHandle)
	{
		return GetTypeHash(InHandle.Handle);
	} */
};

DECLARE_DYNAMIC_DELEGATE(FTimerDynamicDelegate);