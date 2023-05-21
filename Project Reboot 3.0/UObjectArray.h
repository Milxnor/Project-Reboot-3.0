#pragma once

#include "inc.h"

#include "Object.h"
#include "ObjectMacros.h"

struct FUObjectItem
{
	UObject* Object;
	int32 Flags;
	int32 ClusterRootIndex;
	int32 SerialNumber;

	FORCEINLINE bool IsPendingKill() const
	{
		return !!(Flags & int32(EInternalObjectFlags::PendingKill));
	}

	FORCEINLINE void SetFlag(EInternalObjectFlags FlagToSet)
	{
		// static_assert(sizeof(int32) == sizeof(Flags), "Flags must be 32-bit for atomics.");
		int32 StartValue = int32(Flags);

		if ((StartValue & int32(FlagToSet)) == int32(FlagToSet))
		{
			return;
		}

		int32 NewValue = StartValue | int32(FlagToSet);
	}

	FORCEINLINE void SetRootSet()
	{
		SetFlag(EInternalObjectFlags::RootSet);
	}
};

class FFixedUObjectArray
{
	FUObjectItem* Objects;
	int32 MaxElements;
	int32 NumElements;
public:
	FORCEINLINE int32 Num() const { return NumElements; }
	FORCEINLINE int32 Capacity() const { return MaxElements; }
	FORCEINLINE bool IsValidIndex(int32 Index) const { return Index < Num() && Index >= 0; }

	FORCEINLINE FUObjectItem* GetItemByIndex(int32 Index)
	{
		if (!IsValidIndex(Index)) return nullptr;
		return &Objects[Index];
	}

	bool IsValid(UObject* Object)
	{
		int32 Index = Object->InternalIndex;
		if (Index == -1)
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Object is not in global object array"));
			return false;
		}
		if (!IsValidIndex(Index))
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Invalid object index %i"), Index);
			return false;
		}

		FUObjectItem* Slot = GetItemByIndex(Index);
		if (!Slot || Slot->Object == nullptr)
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Empty slot"));
			return false;
		}
		if (Slot->Object != Object)
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Other object in slot"));
			return false;
		}
		return true;
	}

	FORCEINLINE UObject* GetObjectByIndex(int32 Index)
	{
		if (auto Item = GetItemByIndex(Index))
			return Item->Object;

		return nullptr;
	}
};

extern inline int NumElementsPerChunk = 0x10000;

class FChunkedFixedUObjectArray
{
	// enum { NumElementsPerChunk = 64 * 1024, };

	FUObjectItem** Objects;
	FUObjectItem* PreAllocatedObjects;
	int32 MaxElements;
	int32 NumElements;
	int32 MaxChunks;
	int32 NumChunks;
public:
	FORCEINLINE int32 Num() const { return NumElements; }
	FORCEINLINE int32 Capacity() const { return MaxElements; }
	FORCEINLINE bool IsValidIndex(int32 Index) const { return Index < Num() && Index >= 0; }

	FORCEINLINE FUObjectItem* GetItemByIndex(int32 Index)
	{
		if (!IsValidIndex(Index)) return nullptr;

		const int32 ChunkIndex = Index / NumElementsPerChunk;
		const int32 WithinChunkIndex = Index % NumElementsPerChunk;

		// checkf(ChunkIndex < NumChunks, TEXT("ChunkIndex (%d) < NumChunks (%d)"), ChunkIndex, NumChunks);
		// checkf(Index < MaxElements, TEXT("Index (%d) < MaxElements (%d)"), Index, MaxElements);
		FUObjectItem* Chunk = Objects[ChunkIndex];

		if (!Chunk)
			return nullptr;

		return Chunk + WithinChunkIndex;
	}

	bool IsValid(UObject* Object)
	{
		int32 Index = Object->InternalIndex;
		if (Index == -1)
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Object is not in global object array"));
			return false;
		}
		if (!IsValidIndex(Index))
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Invalid object index %i"), Index);
			return false;
		}

		FUObjectItem* Slot = GetItemByIndex(Index);
		if (!Slot || Slot->Object == nullptr)
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Empty slot"));
			return false;
		}
		if (Slot->Object != Object)
		{
			// UE_LOG(LogUObjectArray, Warning, TEXT("Other object in slot"));
			return false;
		}
		return true;
	}

	FORCEINLINE UObject* GetObjectByIndex(int32 Index)
	{
		if (auto Item = GetItemByIndex(Index))
			return Item->Object;

		return nullptr;
	}
};

extern inline FChunkedFixedUObjectArray* ChunkedObjects = 0;
extern inline FFixedUObjectArray* UnchunkedObjects = 0;

FORCEINLINE UObject* GetObjectByIndex(int32 Index)
{
	return ChunkedObjects ? ChunkedObjects->GetObjectByIndex(Index) : UnchunkedObjects ? UnchunkedObjects->GetObjectByIndex(Index) : nullptr;
}

FORCEINLINE FUObjectItem* GetItemByIndex(int32 Index)
{
	return ChunkedObjects ? ChunkedObjects->GetItemByIndex(Index) : UnchunkedObjects ? UnchunkedObjects->GetItemByIndex(Index) : nullptr;
}