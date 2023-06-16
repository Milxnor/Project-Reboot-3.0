#pragma once

#include "inc.h"
#include "addresses.h"

#include "MemoryOps.h"
#include "ContainerAllocationPolicies.h"

struct FMemory
{
	static inline void* (*Realloc)(void* Original, SIZE_T Count, uint32_t Alignment /* = DEFAULT_ALIGNMENT */);
};

template <typename T = __int64>
static T* AllocUnreal(size_t Size)
{
	return (T*)FMemory::Realloc(0, Size, 0);
}

template<typename InElementType> //, typename InAllocatorType>
class TArray
{
	// protected:
public:
	friend class FString;

	using ElementAllocatorType = InElementType*;
	using SizeType = int32;

	ElementAllocatorType Data = nullptr; // AllocatorInstance;
	SizeType             ArrayNum;
	SizeType             ArrayMax;

public:

	inline InElementType& At(int i, size_t Size = sizeof(InElementType)) const { return *(InElementType*)(__int64(Data) + (static_cast<long long>(Size) * i)); }
	inline InElementType& at(int i, size_t Size = sizeof(InElementType)) const { return *(InElementType*)(__int64(Data) + (static_cast<long long>(Size) * i)); }
	inline InElementType* AtPtr(int i, size_t Size = sizeof(InElementType)) const { return (InElementType*)(__int64(Data) + (static_cast<long long>(Size) * i)); }

	bool IsValidIndex(int i) { return i > 0 && i < ArrayNum; }

	ElementAllocatorType& GetData() const { return Data; }
	ElementAllocatorType& GetData() { return Data; }

	void Reserve(int Number, size_t Size = sizeof(InElementType))
	{
		// LOG_INFO(LogDev, "ArrayNum {}", ArrayNum);
		// Data = (InElementType*)FMemory::Realloc(Data, (ArrayMax = ArrayNum + Number) * Size, 0);
		Data = /* (ArrayMax - ArrayNum) >= ArrayNum ? Data : */ (InElementType*)FMemory::Realloc(Data, (ArrayMax = Number + ArrayNum) * Size, 0);
	}

	int CalculateSlackReserve(SizeType NumElements, SIZE_T NumBytesPerElement) const
	{
		return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, false);
	}

	void ResizeArray(SizeType NewNum, SIZE_T NumBytesPerElement)
	{
		const SizeType CurrentMax = ArrayMax;
		SizeType v3 = NewNum;
		if (NewNum)
		{
			/* SizeType v6 = (unsigned __int64)FMemory::QuantizeSize(4 * NewNum, 0) >> 2;
			// if (v3 > (int)v6)
				// LODWORD(v6) = 0x7FFFFFFF;
			v3 = v6; */
		}

		if (v3 != CurrentMax && (Data || v3))
			Data = (InElementType*)FMemory::Realloc(Data, NumBytesPerElement * v3, 0);

		ArrayNum = v3; // ?
		ArrayMax = v3;
	}

	void RefitArray(SIZE_T NumBytesPerElement = sizeof(InElementType))
	{
		auto newNum = ArrayNum;

		// newNum = FMemory::QuantizeSize(NumBytesPerElement * newNum, 0) >> 4

		ArrayMax = newNum;

		if (Data || ArrayNum)
		{
			Data = false ? (InElementType*)VirtualAlloc(0, newNum * NumBytesPerElement, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE) :
				(InElementType*)FMemory::Realloc(Data, newNum * NumBytesPerElement, 0);
		}
	}

	int AddUninitialized2(SIZE_T NumBytesPerElement = sizeof(InElementType))
	{
		const int OldArrayNum = ArrayNum;

		ArrayNum = OldArrayNum + 1;

		if (OldArrayNum + 1 > ArrayMax)
		{
			RefitArray(NumBytesPerElement);
			// ResizeArray(ArrayNum, NumBytesPerElement);
		}

		return OldArrayNum;
	}

	void CopyFromArray(TArray<InElementType>& OtherArray, SIZE_T NumBytesPerElement = sizeof(InElementType))
	{
		if (!OtherArray.ArrayNum && !ArrayMax) // so if the new array has nothing, and we currently have nothing allocated, then we can just return
		{
			ArrayMax = 0;
			return;
		}

		ResizeArray(OtherArray.ArrayNum, NumBytesPerElement);
		memcpy(this->Data, OtherArray.Data, NumBytesPerElement * OtherArray.ArrayNum);
	}

	/*
	FORCENOINLINE void ResizeForCopy(SizeType NewMax, SizeType PrevMax, int ElementSize = sizeof(InElementType))
	{
		if (NewMax)
		{
			NewMax = CalculateSlackReserve(NewMax, ElementSize);
		}
		if (NewMax != PrevMax)
		{
			int ReserveCount = NewMax - PrevMax; // IDK TODO Milxnor
			Reserve(ReserveCount, ElementSize);
			// AllocatorInstance.ResizeAllocation(0, NewMax, ElementSize);
		}

		ArrayMax = NewMax;
	}

	template <typename OtherElementType, typename OtherSizeType>
	void CopyToEmpty(const OtherElementType* OtherData, OtherSizeType OtherNum, SizeType PrevMax, SizeType ExtraSlack, int ElementSize = sizeof(InElementType))
	{
		SizeType NewNum = (SizeType)OtherNum;
		// checkf((OtherSizeType)NewNum == OtherNum, TEXT("Invalid number of elements to add to this array type: %llu"), (unsigned long long)NewNum);

		// checkSlow(ExtraSlack >= 0);
		ArrayNum = NewNum;

		if (OtherNum || ExtraSlack || PrevMax)
		{
			ResizeForCopy(NewNum + ExtraSlack, PrevMax);
			ConstructItems<InElementType>(GetData(), OtherData, OtherNum);
		}
		else
		{
			ArrayMax = 0; // AllocatorInstance.GetInitialCapacity();
		}
	}

	FORCEINLINE TArray(const TArray& Other)
	{
		CopyToEmpty(Other.Data, Other.Num(), 0, 0);
	}
	*/

	TArray() : Data(nullptr), ArrayNum(0), ArrayMax(0) {}

	inline int Num() const { return ArrayNum; }
	inline int size() const { return ArrayNum; }

	/* FORCENOINLINE void ResizeTo(int32 NewMax)
	{
		if (NewMax)
		{
			NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
		}
		if (NewMax != ArrayMax)
		{
			ArrayMax = NewMax;
			AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, sizeof(ElementType));
		}
	}

	void Empty(int32 Slack = 0)
	{
		// DestructItems(GetData(), ArrayNum);

		// checkSlow(Slack >= 0);
		ArrayNum = 0;

		if (ArrayMax != Slack)
		{
			ResizeTo(Slack);
		}
	}

	void Reset(int32 NewSize = 0)
	{
		// If we have space to hold the excepted size, then don't reallocate
		if (NewSize <= ArrayMax)
		{
			// DestructItems(GetData(), ArrayNum);
			ArrayNum = 0;
		}
		else
		{
			Empty(NewSize);
		}
	} */

	void RemoveAtImpl(int32 Index, int32 Count, bool bAllowShrinking)
	{
		if (Count)
		{
			// CheckInvariants();
			// checkSlow((Count >= 0) & (Index >= 0) & (Index + Count <= ArrayNum));

			// DestructItems(GetData() + Index, Count); // TODO milxnor

			// Skip memmove in the common case that there is nothing to move.
			int32 NumToMove = ArrayNum - Index - Count;
			if (NumToMove)
			{
				/* FMemory::Memmove
				(
					(uint8*)AllocatorInstance.GetAllocation() + (Index) * sizeof(ElementType),
					(uint8*)AllocatorInstance.GetAllocation() + (Index + Count) * sizeof(ElementType),
					NumToMove * sizeof(ElementType)
				); */
				// memmove(Data + (Index) * sizeof(InElementType), Data + (Index + Count) * sizeof(InElementType), NumToMove * sizeof(InElementType)); // i think this wrong
			}
			ArrayNum -= Count;

			if (bAllowShrinking)
			{
				// ResizeShrink(); // TODO milxnor
			}
		}
	}


	FORCEINLINE SizeType CalculateSlackGrow(SizeType NumElements, SizeType NumAllocatedElements, SIZE_T NumBytesPerElement) const
	{
		return ArrayMax - NumElements;
	}

	template <typename CountType>
	FORCEINLINE void RemoveAt(int32 Index, CountType Count, bool bAllowShrinking = true)
	{
		// static_assert(!TAreTypesEqual<CountType, bool>::Value, "TArray::RemoveAt: unexpected bool passed as the Count argument");
		RemoveAtImpl(Index, Count, bAllowShrinking);
	}

	FORCENOINLINE void ResizeGrow(int32 OldNum, size_t Size = sizeof(InElementType))
	{
		// LOG_INFO(LogMemory, "FMemory::Realloc: {}", __int64(FMemory::Realloc));

		ArrayMax = ArrayNum; // CalculateSlackGrow(/* ArrayNum */ OldNum, ArrayMax, Size);
		// AllocatorInstance.ResizeAllocation(OldNum, ArrayMax, sizeof(ElementType));
		// LOG_INFO(LogMemory, "ArrayMax: {} Size: {}", ArrayMax, Size);
		Data = (InElementType*)FMemory::Realloc(Data, ArrayNum * Size, 0);
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
	}

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
	}

	void FreeGood(SizeType Size = sizeof(InElementType))
	{
		if (Data)
		{
			if (true)
			{
				static void (*FreeOriginal)(void* Original) = decltype(FreeOriginal)(Addresses::Free);

				if (FreeOriginal)
					FreeOriginal(Data);
			}
			else
			{
				VirtualFree(Data, 0, MEM_RELEASE);
			}
		}

		Data = nullptr;
		ArrayNum = 0;
		ArrayMax = 0;
	}

	void FreeReal(SizeType Size = sizeof(InElementType))
	{
		if (!IsBadReadPtr(Data, 8) && ArrayNum > 0 && sizeof(InElementType) > 0)
		{
			for (int i = 0; i < ArrayNum; ++i)
			{
				auto current = AtPtr(i, Size);

				RtlSecureZeroMemory(current, Size);
			}

			// VirtualFree(Data, _msize(Data), MEM_RELEASE);
			// VirtualFree(Data, sizeof(InElementType) * ArrayNum, MEM_RELEASE); // ik this does nothing

			/* static void (*FreeOriginal)(void*) = decltype(FreeOriginal)(Addresses::Free);

			if (FreeOriginal)
			{
				FreeOriginal(Data);
			}
			else */
			{
				auto res = VirtualFree(Data, 0, MEM_RELEASE);
				// auto res = VirtualFree(Data, sizeof(InElementType) * ArrayNum, MEM_RELEASE);
				LOG_INFO(LogDev, "Free: {} aa: 0x{:x}", res, res ? 0 : GetLastError());
			}
		}

		Data = nullptr;
		ArrayNum = 0;
		ArrayMax = 0;
	}

	void Free()
	{
		if (Data && ArrayNum > 0 && sizeof(InElementType) > 0)
		{
			// VirtualFree(Data, _msize(Data), MEM_RELEASE);
			VirtualFree(Data, sizeof(InElementType) * ArrayNum, MEM_RELEASE); // ik this does nothing
			// VirtualFree(Data, 0, MEM_RELEASE);
		}

		Data = nullptr;
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
	}
};