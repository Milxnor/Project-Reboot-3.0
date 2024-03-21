#pragma once

#include "ContainerAllocationPolicies.h"
#include "UnrealMathUtility.h"

#define NumBitsPerDWORD ((int32)32)
#define NumBitsPerDWORDLogTwo ((int32)5)

template<typename Allocator = FDefaultBitArrayAllocator>
class TBitArray;

template<typename Allocator = FDefaultBitArrayAllocator>
class TConstSetBitIterator;

template<typename Allocator /*= FDefaultBitArrayAllocator*/>
class TBitArray
{
public:
	/**
 * Move constructor.
 */
	FORCEINLINE TBitArray(TBitArray&& Other)
	{
		MoveOrCopy(*this, Other);
	}

	FORCEINLINE int32 Num() const { return NumBits; }

	/**
	 * Copy constructor.
	 */
	FORCEINLINE TBitArray(const TBitArray& Copy)
		: NumBits(0)
		, MaxBits(0)
	{
		*this = Copy;
	}

	FORCEINLINE const uint32* GetData() const
	{
		return (uint32*)AllocatorInstance.GetAllocation();
	}

	FORCEINLINE uint32* GetData()
	{
		return (uint32*)AllocatorInstance.GetAllocation();
	}

	/**
	 * Move assignment.
	 */
	FORCEINLINE TBitArray& operator=(TBitArray&& Other)
	{
		if (this != &Other)
		{
			MoveOrCopy(*this, Other);
		}

		return *this;
	}

private:
	typedef typename Allocator::template ForElementType<uint32> AllocatorType;

	AllocatorType AllocatorInstance;
	int32         NumBits;
	int32         MaxBits;

	template <typename BitArrayType>
	static FORCEINLINE typename TEnableIf<TContainerTraits<BitArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
	{
		ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);

		ToArray.NumBits = FromArray.NumBits;
		ToArray.MaxBits = FromArray.MaxBits;
		FromArray.NumBits = 0;
		FromArray.MaxBits = 0;
	}

	template <typename BitArrayType>
	static FORCEINLINE typename TEnableIf<!TContainerTraits<BitArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
	{
		ToArray = FromArray;
	}

	FORCENOINLINE void Realloc(int32 PreviousNumBits)
	{
		const int32 PreviousNumDWORDs = FMath::DivideAndRoundUp(PreviousNumBits, NumBitsPerDWORD);
		const int32 MaxDWORDs = FMath::DivideAndRoundUp(MaxBits, NumBitsPerDWORD);

		AllocatorInstance.ResizeAllocation(PreviousNumDWORDs, MaxDWORDs, sizeof(uint32));

		if (MaxDWORDs)
		{
			// Reset the newly allocated slack DWORDs.
			FMemory::Memzero((uint32*)AllocatorInstance.GetAllocation() + PreviousNumDWORDs, (MaxDWORDs - PreviousNumDWORDs) * sizeof(uint32));
		}
	}
public:
};

class FRelativeBitReference
{
public:
	FORCEINLINE explicit FRelativeBitReference(int32 BitIndex)
		: DWORDIndex(BitIndex >> NumBitsPerDWORDLogTwo)
		, Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
	{
	}

	int32  DWORDIndex;
	uint32 Mask;
};

template<typename Allocator>
class TConstSetBitIterator : public FRelativeBitReference
{
public:

	/** Constructor. */
	TConstSetBitIterator(const TBitArray<Allocator>& InArray, int32 StartIndex = 0)
		: FRelativeBitReference(StartIndex)
		, Array(InArray)
		, UnvisitedBitMask((~0U) << (StartIndex & (NumBitsPerDWORD - 1)))
		, CurrentBitIndex(StartIndex)
		, BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
	{
		// check(StartIndex >= 0 && StartIndex <= Array.Num());
		if (StartIndex != Array.Num())
		{
			FindFirstSetBit();
		}
	}

	/** Forwards iteration operator. */
	FORCEINLINE TConstSetBitIterator& operator++()
	{
		// Mark the current bit as visited.
		UnvisitedBitMask &= ~this->Mask;

		// Find the first set bit that hasn't been visited yet.
		FindFirstSetBit();

		return *this;
	}

	FORCEINLINE friend bool operator==(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs)
	{
		// We only need to compare the bit index and the array... all the rest of the state is unobservable.
		return Lhs.CurrentBitIndex == Rhs.CurrentBitIndex && &Lhs.Array == &Rhs.Array;
	}

	FORCEINLINE friend bool operator!=(const TConstSetBitIterator& Lhs, const TConstSetBitIterator& Rhs)
	{
		return !(Lhs == Rhs);
	}

	/** conversion to "bool" returning true if the iterator is valid. */
	FORCEINLINE explicit operator bool() const
	{
		return CurrentBitIndex < Array.Num();
	}
	/** inverse of the "bool" operator */
	FORCEINLINE bool operator !() const
	{
		return !(bool)*this;
	}

	/** Index accessor. */
	FORCEINLINE int32 GetIndex() const
	{
		return CurrentBitIndex;
	}

private:

	const TBitArray<Allocator>& Array;

	uint32 UnvisitedBitMask;
	int32 CurrentBitIndex;
	int32 BaseBitIndex;


	/** Find the first set bit starting with the current bit, inclusive. */
	void FindFirstSetBit()
	{
		const uint32* ArrayData = Array.GetData();
		const int32   ArrayNum = Array.Num();
		const int32   LastDWORDIndex = (ArrayNum - 1) / NumBitsPerDWORD;

		// Advance to the next non-zero uint32.
		uint32 RemainingBitMask = ArrayData[this->DWORDIndex] & UnvisitedBitMask;
		while (!RemainingBitMask)
		{
			++this->DWORDIndex;
			BaseBitIndex += NumBitsPerDWORD;
			if (this->DWORDIndex > LastDWORDIndex)
			{
				// We've advanced past the end of the array.
				CurrentBitIndex = ArrayNum;
				return;
			}

			RemainingBitMask = ArrayData[this->DWORDIndex];
			UnvisitedBitMask = ~0;
		}

		// This operation has the effect of unsetting the lowest set bit of BitMask
		const uint32 NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

		// This operation XORs the above mask with the original mask, which has the effect
		// of returning only the bits which differ; specifically, the lowest bit
		this->Mask = NewRemainingBitMask ^ RemainingBitMask;

		// If the Nth bit was the lowest set bit of BitMask, then this gives us N
		CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - FMath::CountLeadingZeros(this->Mask);

		// If we've accidentally iterated off the end of an array but still within the same DWORD
		// then set the index to the last index of the array
		if (CurrentBitIndex > ArrayNum)
		{
			CurrentBitIndex = ArrayNum;
		}
	}
};
