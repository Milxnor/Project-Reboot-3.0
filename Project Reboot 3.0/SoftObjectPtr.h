#pragma once

#include "Object.h"

#include "PersistentObjectPtr.h"
#include "SoftObjectPath.h"
#include "AssetPtr.h"

#include "reboot.h"

struct FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
{
public:
};

template<class T = UObject>
struct TSoftObjectPtr
{
public:
	FSoftObjectPtr SoftObjectPtr;

	T* Get(UClass* ClassToLoad = nullptr, bool bTryToLoad = false)
	{
		if (Engine_Version <= 416)
		{
			auto& AssetPtr = *(TAssetPtr<T>*)this;
			return AssetPtr.Get();
		}
		else
		{
			if (SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value <= 0)
				return nullptr;

			if (bTryToLoad)
			{
				return LoadObject<T>(SoftObjectPtr.ObjectID.AssetPathName.ToString(), ClassToLoad);
			}

			return FindObject<T>(SoftObjectPtr.ObjectID.AssetPathName.ToString());
		}
	}
};