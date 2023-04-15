#pragma once

#include "inc.h"

class FNetworkGUID
{
public:

	uint32 Value;

public:

	FNetworkGUID()
		: Value(0)
	{ }

	FNetworkGUID(uint32 V)
		: Value(V)
	{ }

public:

	friend bool operator==(const FNetworkGUID& X, const FNetworkGUID& Y)
	{
		return (X.Value == Y.Value);
	}

	friend bool operator!=(const FNetworkGUID& X, const FNetworkGUID& Y)
	{
		return (X.Value != Y.Value);
	}

	/* friend FArchive& operator<<(FArchive& Ar, FNetworkGUID& G)
	{
		Ar.SerializeIntPacked(G.Value);
		return Ar;
	} */

public:

	void BuildFromNetIndex(int32 StaticNetIndex)
	{
		Value = (StaticNetIndex << 1 | 1);
	}

	int32 ExtractNetIndex()
	{
		if (Value & 1)
		{
			return Value >> 1;
		}
		return 0;
	}

	friend uint32 GetTypeHash(const FNetworkGUID& Guid)
	{
		return Guid.Value;
	}

	bool IsDynamic() const
	{
		return Value > 0 && !(Value & 1);
	}

	bool IsStatic() const
	{
		return Value & 1;
	}

	bool IsValid() const
	{
		return Value > 0;
	}

	// CORE_API bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	/** A Valid but unassigned NetGUID */
	bool IsDefault() const
	{
		return (Value == 1);
	}

	/* CORE_API*/ static FNetworkGUID GetDefault()
	{
		return FNetworkGUID(1);
	}

	void Reset()
	{
		Value = 0;
	}

	/* FString ToString() const
	{
		return FString::Printf(TEXT("%d"), Value);
	} */

public:

	/* CORE_API */ static FNetworkGUID Make(int32 seed, bool bIsStatic)
	{
		return FNetworkGUID(seed << 1 | (bIsStatic ? 1 : 0));
	}
};
