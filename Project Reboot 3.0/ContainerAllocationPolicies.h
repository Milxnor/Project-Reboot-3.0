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
    };
};