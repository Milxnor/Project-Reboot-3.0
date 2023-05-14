#include "reboot.h"

#include "SoftObjectPtr.h"

#include "KismetStringLibrary.h"

UObject* Assets::LoadAsset(FName Name, bool ShowDelayTimes)
{
	static UObject* (*LoadAssetOriginal)(FName a1, bool a2) = decltype(LoadAssetOriginal)(Assets::LoadAsset);

	return LoadAssetOriginal(Name, ShowDelayTimes);
}

UObject* Assets::LoadSoftObject(void* SoftObjectPtr)
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

	return Assets::LoadAsset(tSoftObjectPtr->SoftObjectPtr.ObjectID.AssetPathName);
}