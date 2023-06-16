#pragma once

#include "NameTypes.h"
#include "Array.h"

struct FGameplayTag
{
	static const int npos = -1; // lol?

	FName TagName;
};

struct FGameplayTagContainer
{
	TArray<FGameplayTag> GameplayTags; 
	TArray<FGameplayTag> ParentTags;

	std::string ToStringSimple(bool bQuoted)
	{
		std::string RetString;
		for (int i = 0; i < GameplayTags.Num(); ++i)
		{
			if (bQuoted)
			{
				RetString += ("\"");
			}
			RetString += GameplayTags.at(i).TagName.ToString();
			if (bQuoted)
			{
				RetString += ("\"");
			}

			if (i < GameplayTags.Num() - 1)
			{
				RetString += (", ");
			}
		}
		return RetString;
	}

	int Find(const std::string& Str)
	{
		for (int i = 0; i < GameplayTags.Num(); ++i)
		{
			if (GameplayTags.at(i).TagName.ToString() == Str)
				return i;
		}

		return FGameplayTag::npos;
	}

	int Find(FGameplayTag& Tag)
	{
		return Find(Tag.TagName.ToString());
	}

	bool Contains(const std::string& Str)
	{
		return Find(Str) != FGameplayTag::npos;
	}

	void Reset()
	{
		GameplayTags.Free();
		ParentTags.Free();
	}
};