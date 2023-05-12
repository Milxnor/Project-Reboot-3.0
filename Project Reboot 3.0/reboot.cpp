#include "reboot.h"

#include "SoftObjectPtr.h"

#include "KismetStringLibrary.h"

inline UObject* Assets::LoadAsset(FName Name, bool ShowDelayTimes)
{
	static UObject* (*LoadAssetOriginal)(FName a1, bool a2);

	return LoadAssetOriginal(Name, ShowDelayTimes);
}

inline UObject* Assets::LoadSoftObject(void* SoftObjectPtr)
{
	if (Engine_Version == 416)
	{
		auto tAssetPtr = (TAssetPtr<UObject>*)SoftObjectPtr;
		// return LoadAsset(tAssetPtr->AssetPtr.ObjectID.AssetLongPathname.);
		return nullptr; // later
	}

	auto tSoftObjectPtr = (TSoftObjectPtr<UObject>*)SoftObjectPtr;

	// if (auto WeakObject = tSoftObjectPtr->GetByWeakObject())
		// return WeakObject;

	return LoadAsset(tSoftObjectPtr->SoftObjectPtr.ObjectID.AssetPathName);
}