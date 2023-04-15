#pragma once

template <typename T>
struct TIsEnum
{
	enum { Value = __is_enum(T) };
};
