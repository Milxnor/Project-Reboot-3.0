#pragma once

#include <type_traits>

template <typename T>
struct TIsTriviallyDestructible
{
	enum { Value = std::is_trivially_destructible_v<T> };
};