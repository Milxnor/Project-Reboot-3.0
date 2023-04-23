#pragma once

template <typename T>
struct TTypeWrapper;

template <typename T>
struct TUnwrapType
{
	typedef T Type;
};

template <typename T>
struct TUnwrapType<TTypeWrapper<T>>
{
	typedef T Type;
};