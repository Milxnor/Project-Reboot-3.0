#pragma once

template <int NumElements>
class TInlineAllocator
{
private:
    template <int Size, int Alignment>
    struct alignas(Alignment) TAlignedBytes
    {
        unsigned char Pad[Size];
    };

    template <typename ElementType>
    struct TTypeCompatibleBytes : public TAlignedBytes<sizeof(ElementType), alignof(ElementType)>
    {
    };

public:
    template <typename ElementType>
    class ForElementType
    {
        friend class TBitArray;

    private:
        TTypeCompatibleBytes<ElementType> InlineData[NumElements];

        ElementType* SecondaryData;

    public:

        FORCEINLINE int32 NumInlineBytes() const
        {
            return sizeof(ElementType) * NumElements;
        }
        FORCEINLINE int32 NumInlineBits() const
        {
            return NumInlineBytes() * 8;
        }

        FORCEINLINE ElementType& operator[](int32 Index)
        {
            return *(ElementType*)(&InlineData[Index]);
        }
        FORCEINLINE const ElementType& operator[](int32 Index) const
        {
            return *(ElementType*)(&InlineData[Index]);
        }

        FORCEINLINE void operator=(void* InElements)
        {
            SecondaryData = InElements;
        }

        FORCEINLINE ElementType& GetInlineElement(int32 Index)
        {
            return *(ElementType*)(&InlineData[Index]);
        }
        FORCEINLINE const ElementType& GetInlineElement(int32 Index) const
        {
            return *(ElementType*)(&InlineData[Index]);
        }
        FORCEINLINE ElementType& GetSecondaryElement(int32 Index)
        {
            return SecondaryData[Index];
        }
        FORCEINLINE const ElementType& GetSecondaryElement(int32 Index) const
        {
            return SecondaryData[Index];
        }
    };
};