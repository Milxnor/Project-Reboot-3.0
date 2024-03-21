#pragma once

#include "Set.h"
#include "UnrealTemplate.h"
#include "ChooseClass.h"

// template <typename KeyType, typename ValueType>
// using TPair = TTuple<KeyType, ValueType>;

template <typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType Key;
	ValueType Value;
};

template <typename KeyInitType, typename ValueInitType>
class TPairInitializer
{
public:
	typename TRValueToLValueReference<KeyInitType  >::Type Key;
	typename TRValueToLValueReference<ValueInitType>::Type Value;

	/** Initialization constructor. */
	FORCEINLINE TPairInitializer(KeyInitType InKey, ValueInitType InValue)
		: Key(InKey)
		, Value(InValue)
	{
	}

	/** Implicit conversion to pair initializer. */
	template <typename KeyType, typename ValueType>
	FORCEINLINE TPairInitializer(const TPair<KeyType, ValueType>& Pair)
		: Key(Pair.Key)
		, Value(Pair.Value)
	{
	}

	template <typename KeyType, typename ValueType>
	operator TPair<KeyType, ValueType>() const
	{
#define StaticCast static_cast // Milxnor: ??
		return TPair<KeyType, ValueType>(StaticCast<KeyInitType>(Key), StaticCast<ValueInitType>(Value));
	}
};

template<typename KeyType, typename ValueType, bool bInAllowDuplicateKeys>
struct TDefaultMapKeyFuncs : BaseKeyFuncs<TPair<KeyType, ValueType>, KeyType, bInAllowDuplicateKeys>
{
	typedef typename TTypeTraits<KeyType>::ConstPointerType KeyInitType;
	typedef const TPairInitializer<typename TTypeTraits<KeyType>::ConstInitType, typename TTypeTraits<ValueType>::ConstInitType>& ElementInitType;

	static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
	{
		return Element.Key;
	}
	static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
	{
		return A == B;
	}
	static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
	{
		return GetTypeHash(Key);
	}
};

template<typename KeyType, typename ValueType, bool bInAllowDuplicateKeys>
struct TDefaultMapHashableKeyFuncs : TDefaultMapKeyFuncs<KeyType, ValueType, bInAllowDuplicateKeys>
{
	// static_assert(THasGetTypeHash<KeyType>::Value, "TMap must have a hashable KeyType unless a custom key func is provided."); // T(R)
};

template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
class TMapBase
{
	template <typename OtherKeyType, typename OtherValueType, typename OtherSetAllocator, typename OtherKeyFuncs>
	friend class TMapBase;

	friend struct TContainerTraits<TMapBase>;

public:
	typedef typename TTypeTraits<KeyType  >::ConstPointerType KeyConstPointerType;
	typedef typename TTypeTraits<KeyType  >::ConstInitType    KeyInitType;
	typedef typename TTypeTraits<ValueType>::ConstInitType    ValueInitType;
	typedef TPair<KeyType, ValueType> ElementType;

protected:
	TMapBase() = default;
	TMapBase(TMapBase&&) = default;
	TMapBase(const TMapBase&) = default;
	TMapBase& operator=(TMapBase&&) = default;
	TMapBase& operator=(const TMapBase&) = default;

	typedef TSet<ElementType, KeyFuncs, SetAllocator> ElementSetType;

	/** The base of TMapBase iterators. */
	template<bool bConst, bool bRangedFor = false>
	class TBaseIterator
	{
	public:
		typedef typename TChooseClass<
			bConst,
			typename TChooseClass<bRangedFor, typename ElementSetType::TRangedForConstIterator, typename ElementSetType::TConstIterator>::Result,
			typename TChooseClass<bRangedFor, typename ElementSetType::TRangedForIterator, typename ElementSetType::TIterator     >::Result
		>::Result PairItType;
	private:
		typedef typename TChooseClass<bConst, const TMapBase, TMapBase>::Result MapType;
		typedef typename TChooseClass<bConst, const KeyType, KeyType>::Result ItKeyType;
		typedef typename TChooseClass<bConst, const ValueType, ValueType>::Result ItValueType;
		typedef typename TChooseClass<bConst, const typename ElementSetType::ElementType, typename ElementSetType::ElementType>::Result PairType;

	public:
		FORCEINLINE TBaseIterator(const PairItType& InElementIt)
			: PairIt(InElementIt)
		{
		}

		FORCEINLINE TBaseIterator& operator++()
		{
			++PairIt;
			return *this;
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{
			return !!PairIt;
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.PairIt == Rhs.PairIt; }
		FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.PairIt != Rhs.PairIt; }

		FORCEINLINE ItKeyType& Key()   const { return PairIt->Key; }
		FORCEINLINE ItValueType& Value() const { return PairIt->Value; }

		FORCEINLINE PairType& operator* () const { return  *PairIt; }
		FORCEINLINE PairType* operator->() const { return &*PairIt; }

	protected:
		PairItType PairIt;
	};

	/** The base type of iterators that iterate over the values associated with a specified key. */
	template<bool bConst>
	class TBaseKeyIterator
	{
	private:
		typedef typename TChooseClass<bConst, typename ElementSetType::TConstKeyIterator, typename ElementSetType::TKeyIterator>::Result SetItType;
		typedef typename TChooseClass<bConst, const KeyType, KeyType>::Result ItKeyType;
		typedef typename TChooseClass<bConst, const ValueType, ValueType>::Result ItValueType;

	public:
		/** Initialization constructor. */
		FORCEINLINE TBaseKeyIterator(const SetItType& InSetIt)
			: SetIt(InSetIt)
		{
		}

		FORCEINLINE TBaseKeyIterator& operator++()
		{
			++SetIt;
			return *this;
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{
			return !!SetIt;
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const
		{
			return !(bool)*this;
		}

		FORCEINLINE ItKeyType& Key() const { return SetIt->Key; }
		FORCEINLINE ItValueType& Value() const { return SetIt->Value; }

	protected:
		SetItType SetIt;
	};

	/** A set of the key-value pairs in the map. */
	ElementSetType Pairs;

public:

	/** Map iterator. */
	class TIterator : public TBaseIterator<false>
	{
	public:

		/** Initialization constructor. */
		FORCEINLINE TIterator(TMapBase& InMap, bool bInRequiresRehashOnRemoval = false)
			: TBaseIterator<false>(InMap.Pairs.CreateIterator())
			, Map(InMap)
			, bElementsHaveBeenRemoved(false)
			, bRequiresRehashOnRemoval(bInRequiresRehashOnRemoval)
		{
		}

		/** Destructor. */
		FORCEINLINE ~TIterator()
		{
			if (bElementsHaveBeenRemoved && bRequiresRehashOnRemoval)
			{
				Map.Pairs.Relax();
			}
		}

		/** Removes the current pair from the map. */
		FORCEINLINE void RemoveCurrent()
		{
			TBaseIterator<false>::PairIt.RemoveCurrent();
			bElementsHaveBeenRemoved = true;
		}

	private:
		TMapBase& Map;
		bool      bElementsHaveBeenRemoved;
		bool      bRequiresRehashOnRemoval;
	};

	/** Const map iterator. */
	class TConstIterator : public TBaseIterator<true>
	{
	public:
		FORCEINLINE TConstIterator(const TMapBase& InMap)
			: TBaseIterator<true>(InMap.Pairs.CreateConstIterator())
		{
		}
	};

	using TRangedForIterator = TBaseIterator<false, true>;
	using TRangedForConstIterator = TBaseIterator<true, true>;

	/** Iterates over values associated with a specified key in a const map. */
	class TConstKeyIterator : public TBaseKeyIterator<true>
	{
	public:
		FORCEINLINE TConstKeyIterator(const TMapBase& InMap, KeyInitType InKey)
			: TBaseKeyIterator<true>(typename ElementSetType::TConstKeyIterator(InMap.Pairs, InKey))
		{}
	};

	/** Iterates over values associated with a specified key in a map. */
	class TKeyIterator : public TBaseKeyIterator<false>
	{
	public:
		FORCEINLINE TKeyIterator(TMapBase& InMap, KeyInitType InKey)
			: TBaseKeyIterator<false>(typename ElementSetType::TKeyIterator(InMap.Pairs, InKey))
		{}

		/** Removes the current key-value pair from the map. */
		FORCEINLINE void RemoveCurrent()
		{
			TBaseKeyIterator<false>::SetIt.RemoveCurrent();
		}
	};

	/** Creates an iterator over all the pairs in this map */
	FORCEINLINE TIterator CreateIterator()
	{
		return TIterator(*this);
	}

	/** Creates a const iterator over all the pairs in this map */
	FORCEINLINE TConstIterator CreateConstIterator() const
	{
		return TConstIterator(*this);
	}

	/** Creates an iterator over the values associated with a specified key in a map */
	FORCEINLINE TKeyIterator CreateKeyIterator(KeyInitType InKey)
	{
		return TKeyIterator(*this, InKey);
	}

	/** Creates a const iterator over the values associated with a specified key in a map */
	FORCEINLINE TConstKeyIterator CreateConstKeyIterator(KeyInitType InKey) const
	{
		return TConstKeyIterator(*this, InKey);
	}

	FORCEINLINE ValueType* Find(KeyConstPointerType Key)
	{
		if (auto* Pair = Pairs.Find(Key))
		{
			return &Pair->Value;
		}

		return nullptr;
	}

	FORCEINLINE const ValueType& FindChecked(KeyConstPointerType Key) const
	{
		const auto* Pair = Pairs.Find(Key);
		// check(Pair != nullptr);
		return Pair->Value;
	}

	/**
	 * Find a reference to the value associated with a specified key.
	 *
	 * @param Key The key to search for.
	 * @return The value associated with the specified key, or triggers an assertion if the key does not exist.
	 */
	FORCEINLINE ValueType& FindChecked(KeyConstPointerType Key)
	{
		auto* Pair = Pairs.Find(Key);
		// check(Pair != nullptr);
		return Pair->Value;
	}

	FORCEINLINE ValueType FindRef(KeyConstPointerType Key) const
	{
		if (const auto* Pair = Pairs.Find(Key))
		{
			return Pair->Value;
		}

		return ValueType();
	}

	FORCEINLINE int32 Num() const
	{
		return Pairs.Num();
	}

private:
	/**
	 * DO NOT USE DIRECTLY
	 * STL-like iterators to enable range-based for loop support.
	 */
	FORCEINLINE friend TRangedForIterator      begin(TMapBase& MapBase) { return TRangedForIterator(begin(MapBase.Pairs)); }
	FORCEINLINE friend TRangedForConstIterator begin(const TMapBase& MapBase) { return TRangedForConstIterator(begin(MapBase.Pairs)); }
	FORCEINLINE friend TRangedForIterator      end(TMapBase& MapBase) { return TRangedForIterator(end(MapBase.Pairs)); }
	FORCEINLINE friend TRangedForConstIterator end(const TMapBase& MapBase) { return TRangedForConstIterator(end(MapBase.Pairs)); }
};

template <typename KeyType, typename ValueType, typename SetAllocator, typename KeyFuncs>
class TSortableMapBase : public TMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
{
	// friend struct TContainerTraits<TSortableMapBase>;

protected:
	typedef TMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;

	TSortableMapBase() = default;
	TSortableMapBase(TSortableMapBase&&) = default;
	TSortableMapBase(const TSortableMapBase&) = default;
	TSortableMapBase& operator=(TSortableMapBase&&) = default;
	TSortableMapBase& operator=(const TSortableMapBase&) = default;
};

template<typename KeyType, typename ValueType, typename SetAllocator = FDefaultSetAllocator, typename KeyFuncs = TDefaultMapHashableKeyFuncs<KeyType,ValueType,false>>
class TMap : public TSortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs>
{
public:
	typedef TSortableMapBase<KeyType, ValueType, SetAllocator, KeyFuncs> Super;
	typedef typename Super::KeyInitType KeyInitType;
	typedef typename Super::KeyConstPointerType KeyConstPointerType;

    typedef TPair<KeyType, ValueType> ElementType;
public:
	typedef TSet<ElementType, KeyFuncs, SetAllocator> ElementSetType;

public:
	TMap() = default;
	TMap(TMap&&) = default;
	TMap(const TMap&) = default;
	TMap& operator=(TMap&&) = default;
	TMap& operator=(const TMap&) = default;
};