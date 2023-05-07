#pragma once

#include "reboot.h"

class UAttributeSet : public UObject
{
public:
};

struct FGameplayAttribute
{
	FString AttributeName; 
	void* Attribute; // Property
	UStruct* AttributeOwner;

	std::string GetAttributeName()
	{
		return AttributeName.ToString();
	}

	std::string GetAttributePropertyName()
	{
		if (!Attribute)
			return "INVALIDATTRIBUTE";

		FName* NamePrivate = nullptr;

		if (Engine_Version >= 425)
			NamePrivate = (FName*)(__int64(Attribute) + 0x28);
		else
			NamePrivate = &((UField*)Attribute)->NamePrivate;

		return NamePrivate->ToString();
	}
};