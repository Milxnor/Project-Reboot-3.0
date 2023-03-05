#pragma once

#include "NameTypes.h"
#include "UnrealString.h"

struct FSoftObjectPath
{
public:
	/** Asset path, patch to a top level object in a package. This is /package/path.assetname */
	FName AssetPathName;

	/** Optional FString for subobject within an asset. This is the sub path after the : */
	FString SubPathString;
};