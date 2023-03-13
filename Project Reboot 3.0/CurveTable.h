#pragma once

#include "Object.h"
#include "NameTypes.h"

#include "reboot.h"

class UCurveTable : public UObject
{
public:
};

struct FCurveTableRowHandle
{
	UCurveTable* CurveTable;
	FName RowName; 
};

struct FSimpleCurveKey
{
	float                                              Time;                                                     // 0x0000(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              Value;                                                    // 0x0004(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FIndexedCurve
{
	
};

struct FRealCurve : public FIndexedCurve
{
};

struct FSimpleCurve : public FRealCurve
{
	TArray<FSimpleCurveKey>& GetKeys()
	{
		static auto KeysOffset = FindOffsetStruct("/Script/Engine.SimpleCurve", "Keys");
		return *(TArray<FSimpleCurveKey>*)(__int64(this) + KeysOffset);
	}
};