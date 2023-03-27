#pragma once

#include "PersistentObjectPtr.h"
#include "StringAssetReference.h"

#include "reboot.h"

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference>
{
public:
};

template<class T = UObject>
class TAssetPtr
{
public:
	FAssetPtr AssetPtr;

	T* Get()
	{
		if (!AssetPtr.ObjectID.AssetLongPathname.IsValid())
			return nullptr;

		return FindObject<T>(AssetPtr.ObjectID.AssetLongPathname.ToString());
	}
};