#pragma once

#include "Delegate.h"
#include "ObjectMacros.h"

#define DECLARE_DELEGATE( DelegateName ) FUNC_DECLARE_DELEGATE( DelegateName, void )
#define DECLARE_DYNAMIC_DELEGATE( DelegateName ) /* BODY_MACRO_COMBINE(CURRENT_FILE_ID,_,__LINE__,_DELEGATE) */ FUNC_DECLARE_DYNAMIC_DELEGATE( FWeakObjectPtr, DelegateName, DelegateName##_DelegateWrapper, , FUNC_CONCAT( *this ), void )
