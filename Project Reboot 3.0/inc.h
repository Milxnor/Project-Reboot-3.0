#pragma once

#include <Windows.h>
#include <iostream>
#include <format>
#include <string>

typedef unsigned short uint16;
typedef unsigned char uint8;
typedef char int8;
typedef short int16;
typedef int int32;
typedef __int64 int64;
typedef unsigned int uint32;
typedef char ANSICHAR;
typedef uint32_t CodeSkipSizeType;
typedef unsigned __int64 uint64;

extern inline int Engine_Version = 0; // For example, 420, 421, etc. // Prevent using this when possible.
extern inline double Fortnite_Version = 0; // For example, 4.1, 6.21, etc. // Prevent using this when possible.
extern inline int Fortnite_CL = 0;

#define PROD // this doesnt do anything besides remove processeventhook and some assert stuff

struct PlaceholderBitfield
{
	uint8_t First : 1;
	uint8_t Second : 1;
	uint8_t Third : 1;
	uint8_t Fourth : 1;
	uint8_t Fifth : 1;
	uint8_t Sixth : 1;
	uint8_t Seventh : 1;
	uint8_t Eighth : 1;
};

#define MS_ALIGN(n) __declspec(align(n))
#define FORCENOINLINE __declspec(noinline)	

#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
	inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
	inline constexpr bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
	inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }

#define UNLIKELY(x)			(x)

inline bool AreVehicleWeaponsEnabled()
{
	return Fortnite_Version > 6;
}

inline bool IsRestartingSupported()
{
	return Engine_Version >= 419 && Engine_Version < 424;
}

// #define ABOVE_S20

/*

enum class AllocatorType
{
	VIRTUALALLOC,
	FMEMORY
};

class InstancedAllocator
{
public:
	AllocatorType allocatorType;

	static void* Allocate(AllocatorType type, size_t Size);

	void* Allocate(size_t Size)
	{
		return Allocate(allocatorType, Size);
	}
};

*/