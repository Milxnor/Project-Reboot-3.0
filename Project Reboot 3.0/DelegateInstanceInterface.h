#pragma once

template <bool Const, typename Class, typename FuncType>
struct TMemFunPtrType;

template <typename Class, typename RetType, typename... ArgTypes>
struct TMemFunPtrType<false, Class, RetType(ArgTypes...)>
{
	typedef RetType(Class::* Type)(ArgTypes...);
};

template <typename Class, typename RetType, typename... ArgTypes>
struct TMemFunPtrType<true, Class, RetType(ArgTypes...)>
{
	typedef RetType(Class::* Type)(ArgTypes...) const;
};