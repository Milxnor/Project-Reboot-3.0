#pragma once

#include "inc.h"

#include "Object.h"

struct FUObjectItem
{
	UObject* Object;
	int32 Flags;
	int32 ClusterRootIndex;
	int32 SerialNumber;
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

	FORCEINLINE UObject* GetObjectByIndex(int32 Index)
	{
		if (auto Item = GetItemByIndex(Index))
			return Item->Object;

		return nullptr;
	}
};

class FChunkedFixedUObjectArray
{
	enum { NumElementsPerChunk = 64 * 1024, };

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
	return ChunkedObjects ? ChunkedObjects->GetObjectByIndex(Index) : UnchunkedObjects->GetObjectByIndex(Index);
}