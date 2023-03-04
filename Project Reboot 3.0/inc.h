#pragma once

#include <Windows.h>
#include <iostream>
#include <format>
#include <string>

typedef unsigned short uint16;
typedef unsigned char uint8;
typedef int int32;
typedef __int64 int64;
typedef unsigned int uint32;
typedef unsigned __int64 uint64;

extern inline int Engine_Version = 0; // For example, 420, 421, etc. // Prevent using this when possible.
extern inline double Fortnite_Version = 0; // For example, 4.1, 6.21, etc. // Prevent using this when possible.

#define MS_ALIGN(n) __declspec(align(n))
#define FORCENOINLINE __declspec(noinline)	