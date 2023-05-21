#include "NameTypes.h"

#include "reboot.h"
#include "UnrealString.h"

#include "KismetStringLibrary.h"

std::string FName::ToString() const
{
	static auto KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");

	static auto Conv_NameToString = FindObject<UFunction>(L"/Script/Engine.KismetStringLibrary.Conv_NameToString");

	struct { FName InName; FString OutStr; } Conv_NameToString_Params{ *this };

	KismetStringLibrary->ProcessEvent(Conv_NameToString, &Conv_NameToString_Params);

	auto Str = Conv_NameToString_Params.OutStr.ToString();
	
	// Conv_NameToString_Params.OutStr.Free();

	return Str;
}

int32 FName::Compare(const FName& Other) const
{
	if (GetComparisonIndexFast() == Other.GetComparisonIndexFast())
	{
		return GetNumber() - Other.GetNumber();
	}
	// Names don't match. This means we don't even need to check numbers.
	else
	{
		// return ToString() == Other.ToString(); // FOR REAL!! (Milxnor)
		/* TNameEntryArray& Names = GetNames();
		const FNameEntry* const ThisEntry = GetComparisonNameEntry();
		const FNameEntry* const OtherEntry = Other.GetComparisonNameEntry();

		FNameBuffer TempBuffer1;
		FNameBuffer TempBuffer2;

		// If one or both entries return an invalid name entry, the comparison fails - fallback to comparing the index
		if (ThisEntry == nullptr || OtherEntry == nullptr)
		{
			return GetComparisonIndexFast() - Other.GetComparisonIndexFast();
		}
		// Ansi/Wide mismatch, convert to wide
		else if (ThisEntry->IsWide() != OtherEntry->IsWide())
		{
			return FCStringWide::Stricmp(
				ThisEntry->IsWide() ? ThisEntry->GetWideNamePtr(TempBuffer1.WideName) : StringCast<WIDECHAR>(ThisEntry->GetAnsiNamePtr(TempBuffer1.AnsiName)).Get(),
				OtherEntry->IsWide() ? OtherEntry->GetWideNamePtr(TempBuffer2.WideName) : StringCast<WIDECHAR>(OtherEntry->GetAnsiNamePtr(TempBuffer2.AnsiName)).Get());
		}
		// Both are wide.
		else if (ThisEntry->IsWide())
		{
			return FCStringWide::Stricmp(ThisEntry->GetWideNamePtr(TempBuffer1.WideName), OtherEntry->GetWideNamePtr(TempBuffer2.WideName));
		}
		// Both are ansi.
		else
		{
			return FCStringAnsi::Stricmp(ThisEntry->GetAnsiNamePtr(TempBuffer1.AnsiName), OtherEntry->GetAnsiNamePtr(TempBuffer2.AnsiName));
		} */
	}

	return GetComparisonIndexFast() < Other.GetComparisonIndexFast();
}

std::string FName::ToString()
{
	static auto KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");

	static auto Conv_NameToString = FindObject<UFunction>(L"/Script/Engine.KismetStringLibrary.Conv_NameToString");

	struct { FName InName; FString OutStr; } Conv_NameToString_Params{ *this };

	KismetStringLibrary->ProcessEvent(Conv_NameToString, &Conv_NameToString_Params);

	auto Str = Conv_NameToString_Params.OutStr.ToString();

	// Conv_NameToString_Params.OutStr.Free();

	return Str;
}