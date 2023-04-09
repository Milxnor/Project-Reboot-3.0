#pragma once

#include "Object.h"
#include "UnrealString.h"
#include "Text.h"

class UKismetTextLibrary : public UObject
{
public:
	static FText Conv_StringToText(const FString& inString);

	static UClass* StaticClass();
};