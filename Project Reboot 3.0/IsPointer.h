#pragma once

template <typename T>
struct TIsPointer
{
	enum { Value = false };
};

template <typename T> struct TIsPointer<T*> { enum { Value = true }; };

template <typename T> struct TIsPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };