#pragma once

#include "inc.h"

struct FMemory
{
	static inline void* (*Realloc)(void* Original, SIZE_T Count, uint32_t Alignment /* = DEFAULT_ALIGNMENT */);
};

template<typename InElementType> //, typename InAllocatorType>
class TArray
{
// protected:
public:
	friend class FString;

	using ElementAllocatorType = InElementType*;
	using SizeType = int32;

	ElementAllocatorType Data = nullptr;// AllocatorInstance;
	SizeType             ArrayNum;
	SizeType             ArrayMax;

public:

	inline InElementType& At(int i, size_t Size = sizeof(InElementType)) const { return *(InElementType*)(__int64(Data) + (static_cast<long long>(Size) * i)); }
	inline InElementType& at(int i, size_t Size = sizeof(InElementType)) const { return *(InElementType*)(__int64(Data) + (static_cast<long long>(Size) * i)); }
	inline InElementType* AtPtr(int i, size_t Size = sizeof(InElementType)) const { return (InElementType*)(__int64(Data) + (static_cast<long long>(Size) * i)); }

	inline int Num() const { return ArrayNum; }
	inline int size() const { return ArrayNum; }

	FORCEINLINE SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, SIZE_T NumBytesPerElement) const
	{
		return ArrayMax - NumElements;
	}

	void Reserve(int Number, size_t Size = sizeof(InElementType))
	{
		// LOG_INFO(LogDev, "ArrayNum {}", ArrayNum);
		// Data = (InElementType*)FMemory::Realloc(Data, (ArrayMax = ArrayNum + Number) * Size, 0);
		Data = /* (ArrayMax - ArrayNum) >= ArrayNum ? Data : */ (InElementType*)FMemory::Realloc(Data, (ArrayMax = Number + ArrayNum) * Size, 0);
	}

	FORCENOINLINE void ResizeGrow(int32 OldNum, size_t Size = sizeof(InElementType))
	{
		// LOG_INFO(LogMemory, "FMemory::Realloc: {}", __int64(FMemory::Realloc));

		ArrayMax = ArrayNum; // CalculateSlackGrow(/* ArrayNum */ OldNum, ArrayMax, Size);
		// AllocatorInstance.ResizeAllocation(OldNum, ArrayMax, sizeof(ElementType));
		// LOG_INFO(LogMemory, "ArrayMax: {} Size: {}", ArrayMax, Size);
		Data = (InElementType*)FMemory::Realloc(Data, ArrayMax * Size, 0);
	}

	FORCEINLINE int32 AddUninitialized(int32 Count = 1, size_t Size = sizeof(InElementType))
	{
		// CheckInvariants();

		if (Count < 0)
		{
			return 0;
		}

		const int32 OldNum = ArrayNum;
		if ((ArrayNum += Count) > ArrayMax)
		{
			ResizeGrow(OldNum, Size);
		}
		return OldNum;
	}

	FORCEINLINE int32 Emplace(const InElementType& New, size_t Size = sizeof(InElementType))
	{
		const int32 Index = AddUninitialized(1, Size); // resizes array
		memcpy_s((InElementType*)(__int64(Data) + (Index * Size)), Size, (void*)&New, Size);
		// new(GetData() + Index) ElementType(Forward<ArgsType>(Args)...);
		return Index;
	}

	/* int Add(const InElementType& New, int Size = sizeof(InElementType))
	{
		return Emplace(New, Size);
	} */

	int AddPtr(InElementType* New, size_t Size = sizeof(InElementType))
	{
		// LOG_INFO(LogDev, "ArrayMax: {}", ArrayMax);

		if ((ArrayNum + 1) > ArrayMax)
		{
			Reserve(1, Size);
		}

		if (Data)
		{
			memcpy_s((InElementType*)(__int64(Data) + (ArrayNum * Size)), Size, (void*)New, Size);
			++ArrayNum;
			return ArrayNum; // - 1;
		}

		return -1;
	};

	int Add(const InElementType& New, size_t Size = sizeof(InElementType))
	{
		// LOG_INFO(LogDev, "ArrayMax: {}", ArrayMax);

		if ((ArrayNum + 1) > ArrayMax)
		{
			Reserve(1, Size);
		}

		if (Data)
		{
			memcpy_s((InElementType*)(__int64(Data) + (ArrayNum * Size)), Size, (void*)&New, Size);
			++ArrayNum;
			return ArrayNum; // - 1;
		}

		return -1;
	};

	void Free()
	{
		if (Data && ArrayNum > 0 && sizeof(InElementType) > 0)
		{
			// VirtualFree(Data, _msize(Data), MEM_RELEASE);
			VirtualFree(Data, sizeof(InElementType) * ArrayNum, MEM_RELEASE);
		}

		ArrayNum = 0;
		ArrayMax = 0;
	}

	bool Remove(const int Index, size_t Size = sizeof(InElementType))
	{
		// return false;

		if (Index < ArrayNum)
		{
			if (Index != ArrayNum - 1)
			{
				memcpy_s(&at(Index, Size), Size, &at(ArrayNum - 1, Size), Size);
				// Data[Index] = Data[ArrayNum - 1];
			}

			--ArrayNum;

			return true;
		}

		return false;
	};
};