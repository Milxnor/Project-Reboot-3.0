#pragma once

#include "inc.h"

struct FRandomStream
{
public:

	/**
	 * Default constructor.
	 *
	 * The seed should be set prior to use.
	 */
	FRandomStream()
		: InitialSeed(0)
		, Seed(0)
	{ }

	/**
	 * Creates and initializes a new random stream from the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	FRandomStream(int32 InSeed)
	{
		Initialize(InSeed);
	}

public:

	/**
	 * Initializes this random stream with the specified seed value.
	 *
	 * @param InSeed The seed value.
	 */
	void Initialize(int32 InSeed)
	{
		InitialSeed = InSeed;
		Seed = uint32(InSeed);
	}

	float GetFraction() const
	{
		MutateSeed();

		float Result;

		*(uint32*)&Result = 0x3F800000U | (Seed >> 9);

		return Result - 1.0f;
	}

	FORCEINLINE float FRand() const
	{
		return GetFraction();
	}

protected:

	/**
	 * Mutates the current seed into the next seed.
	 */
	void MutateSeed() const
	{
		Seed = (Seed * 196314165U) + 907633515U;
	}

private:

	// Holds the initial seed.
	int32 InitialSeed;

	// Holds the current seed. This should be an uint32 so that any shift to obtain top bits
	// is a logical shift, rather than an arithmetic shift (which smears down the negative bit).
	mutable uint32 Seed;
};