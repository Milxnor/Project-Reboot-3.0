#pragma once

#include "DelegateBase.h"
#include "ScriptDelegates.h"

#include "DelegateInstanceInterface.h"
#include "RemoveReference.h"
#include "TypeWrapper.h"

// template <typename WrappedRetValType, typename... ParamTypes> // (Milxnor) IDK IM SCUFFED
class TBaseDelegate : public FDelegateBase
{
public:
	// (Milxnor) YEAH NO

	/*
	typedef typename TUnwrapType<WrappedRetValType>::Type RetValType;

	template <typename UserClass, typename... VarTypes>
	FUNCTION_CHECK_RETURN_START
		inline static TBaseDelegate<RetValType, ParamTypes...> CreateRaw(UserClass* InUserObject, typename TMemFunPtrType<false, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
		FUNCTION_CHECK_RETURN_END
	{
		// UE_STATIC_DEPRECATE(4.23, TIsConst<UserClass>::Value, "Binding a delegate with a const object pointer and non-const function is deprecated.");

		TBaseDelegate<RetValType, ParamTypes...> Result;
		TBaseRawMethodDelegateInstance<false, UserClass, TFuncType, VarTypes...>::Create(Result, InUserObject, InFunc, Vars...);
		return Result;
	}
	template <typename UserClass, typename... VarTypes>
	FUNCTION_CHECK_RETURN_START
		inline static TBaseDelegate<RetValType, ParamTypes...> CreateRaw(UserClass* InUserObject, typename TMemFunPtrType<true, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
		FUNCTION_CHECK_RETURN_END
	{
		TBaseDelegate<RetValType, ParamTypes...> Result;
		TBaseRawMethodDelegateInstance<true, UserClass, TFuncType, VarTypes...>::Create(Result, InUserObject, InFunc, Vars...);
		return Result;
	}


	template <typename UserClass, typename... VarTypes>
	inline void BindRaw(UserClass* InUserObject, typename TMemFunPtrType<false, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
	{
		// UE_STATIC_DEPRECATE(4.23, TIsConst<UserClass>::Value, "Binding a delegate with a const object pointer and non-const function is deprecated.");

		*this = CreateRaw(const_cast<typename TRemoveConst<UserClass>::Type*>(InUserObject), InFunc, Vars...);
	}
	template <typename UserClass, typename... VarTypes>
	inline void BindRaw(UserClass* InUserObject, typename TMemFunPtrType<true, UserClass, RetValType(ParamTypes..., VarTypes...)>::Type InFunc, VarTypes... Vars)
	{
		*this = CreateRaw(InUserObject, InFunc, Vars...);
	} */
};

template <typename TWeakPtr, typename RetValType, typename... ParamTypes>
class TBaseDynamicDelegate : public TScriptDelegate<TWeakPtr>
{
public:
	/**
	 * Default constructor
	 */
	TBaseDynamicDelegate() { }
};