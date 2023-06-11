#pragma once

#include "inc.h"

struct FVector
{
public:
#ifdef ABOVE_S20
	using VectorDataType = double;
#else
	using VectorDataType = float;
#endif

	VectorDataType X;
	VectorDataType Y;
	VectorDataType Z;

	bool CompareVectors(const FVector& A)
	{
		return X == A.X && Y == A.Y && Z == A.Z;
	}

	FVector() : X(0), Y(0), Z(0) {}
	FVector(VectorDataType x, VectorDataType y, VectorDataType z) : X(x), Y(y), Z(z) {}

	FVector operator+(const FVector& A)
	{
		return FVector{ this->X + A.X, this->Y + A.Y, this->Z + A.Z };
	}

	FVector operator-(const FVector& A)
	{
		return FVector{ this->X - A.X, this->Y - A.Y, this->Z - A.Z };
	}

	FORCEINLINE VectorDataType SizeSquared() const
	{
		return X * X + Y * Y + Z * Z;
	}

	FORCEINLINE VectorDataType operator|(const FVector& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	FVector operator*(const VectorDataType A)
	{
		return FVector{ this->X * A, this->Y * A, this->Z * A };
	}

	/* bool operator==(const FVector& A)
	{
		return X == A.X && Y == A.Y && Z == A.Z;
	} */

	void operator+=(const FVector& A)
	{
		*this = *this + A;
	}

	void operator-=(const FVector& A)
	{
		*this = *this - A;
	}
};