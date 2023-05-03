#pragma once

#include "reboot.h"

class UAttributeSet : public UObject
{
public:
};

struct FGameplayAttribute
{
	FString AttributeName; 
	void* Attribute; 
	UStruct* AttributeOwner; 
};