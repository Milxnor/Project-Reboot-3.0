#pragma once

#include "WeakObjectPtr.h"

template<class TObjectID>
struct TPersistentObjectPtr
{
public:
	/** Once the object has been noticed to be loaded, this is set to the object weak pointer **/
	mutable FWeakObjectPtr	WeakPtr;
	/** Compared to CurrentAnnotationTag and if they are not equal, a guid search will be performed **/
	mutable int			TagAtLastTest;
	/** Guid for the object this pointer points to or will point to. **/
	TObjectID				ObjectID;
};