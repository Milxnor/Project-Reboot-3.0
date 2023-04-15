#pragma once

#include "inc.h"

#include "Decay.h"
#include "PointerIsConvertibleFromTo.h"
#include "EnableIf.h"

namespace UE4Invoke_Private
{
	template <typename BaseType, typename CallableType>
	FORCEINLINE auto DereferenceIfNecessary(CallableType&& Callable)
		-> typename TEnableIf<TPointerIsConvertibleFromTo<typename TDecay<CallableType>::Type, typename TDecay<BaseType>::Type>::Value, decltype((CallableType&&)Callable)>::Type
	{
		return (CallableType&&)Callable;
	}

	template <typename BaseType, typename CallableType>
	FORCEINLINE auto DereferenceIfNecessary(CallableType&& Callable)
		-> typename TEnableIf<!TPointerIsConvertibleFromTo<typename TDecay<CallableType>::Type, typename TDecay<BaseType>::Type>::Value, decltype(*(CallableType&&)Callable)>::Type
	{
		return *(CallableType&&)Callable;
	}
}

template <typename FuncType, typename... ArgTypes>
FORCEINLINE auto Invoke(FuncType&& Func, ArgTypes&&... Args)
-> decltype(Forward<FuncType>(Func)(Forward<ArgTypes>(Args)...))
{
	return Forward<FuncType>(Func)(Forward<ArgTypes>(Args)...);
}

template <typename ReturnType, typename ObjType, typename CallableType>
FORCEINLINE auto Invoke(ReturnType ObjType::* pdm, CallableType&& Callable)
-> decltype(UE4Invoke_Private::DereferenceIfNecessary<ObjType>(Forward<CallableType>(Callable)).*pdm)
{
	return UE4Invoke_Private::DereferenceIfNecessary<ObjType>(Forward<CallableType>(Callable)).*pdm;
}

template <typename ReturnType, typename ObjType, typename... PMFArgTypes, typename CallableType, typename... ArgTypes>
FORCEINLINE auto Invoke(ReturnType(ObjType::* PtrMemFun)(PMFArgTypes...), CallableType&& Callable, ArgTypes&&... Args)
-> decltype((UE4Invoke_Private::DereferenceIfNecessary<ObjType>(Forward<CallableType>(Callable)).*PtrMemFun)(Forward<ArgTypes>(Args)...))
{
	return (UE4Invoke_Private::DereferenceIfNecessary<ObjType>(Forward<CallableType>(Callable)).*PtrMemFun)(Forward<ArgTypes>(Args)...);
}

template <typename ReturnType, typename ObjType, typename... PMFArgTypes, typename CallableType, typename... ArgTypes>
FORCEINLINE auto Invoke(ReturnType(ObjType::* PtrMemFun)(PMFArgTypes...) const, CallableType&& Callable, ArgTypes&&... Args)
-> decltype((UE4Invoke_Private::DereferenceIfNecessary<ObjType>(Forward<CallableType>(Callable)).*PtrMemFun)(Forward<ArgTypes>(Args)...))
{
	return (UE4Invoke_Private::DereferenceIfNecessary<ObjType>(Forward<CallableType>(Callable)).*PtrMemFun)(Forward<ArgTypes>(Args)...);
}
