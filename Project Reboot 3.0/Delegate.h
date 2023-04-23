#pragma once

#include "DelegateBase.h"
#include "DelegateSignatureImpl.inl"

#define FUNC_CONCAT( ... ) __VA_ARGS__

#define FUNC_DECLARE_DELEGATE( DelegateName, ReturnType, ... ) \
	typedef TBaseDelegate/*<__VA_ARGS__>*/ DelegateName;

#define FUNC_DECLARE_DYNAMIC_DELEGATE( TWeakPtr, DynamicDelegateName, ExecFunction, FuncParamList, FuncParamPassThru, ... ) \
	class DynamicDelegateName : public TBaseDynamicDelegate<TWeakPtr, __VA_ARGS__> \
	{ \
	public: \
		DynamicDelegateName() \
		{ \
		} \
		\
	};