#pragma once

struct FVector
{
public:
	float X;
	float Y;
	float Z;

	bool CompareVectors(const FVector& A)
	{
		return X == A.X && Y == A.Y && Z == A.Z;
	}

	FVector() : X(0), Y(0), Z(0) {}
	FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

	FVector operator+(const FVector& A)
	{
		return FVector{ this->X + A.X, this->Y + A.Y, this->Z + A.Z };
	}

	FVector operator-(const FVector& A)
	{
		return FVector{ this->X - A.X, this->Y - A.Y, this->Z - A.Z };
	}

	FORCEINLINE float SizeSquared() const
	{
		return X * X + Y * Y + Z * Z;
	}

	FORCEINLINE float operator|(const FVector& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	FVector operator*(const float A)
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