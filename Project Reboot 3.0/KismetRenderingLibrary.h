#pragma once

#include "Object.h"
#include "UnrealString.h"

class UKismetRenderingLibrary : public UObject // UBlueprintFunctionLibrary
{
public:
	static void ExportTexture2D(UObject* WorldContextObject, class UTexture2D* Texture, const FString& FilePath, const FString& Filename);

	static UClass* StaticClass();
};