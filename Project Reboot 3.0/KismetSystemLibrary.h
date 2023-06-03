#pragma once

#include "Object.h"
#include "UnrealString.h"

#include "reboot.h"
#include "Stack.h"

enum class EDrawDebugTrace : uint8_t
{
	None = 0,
	ForOneFrame = 1,
	ForDuration = 2,
	Persistent = 3,
	EDrawDebugTrace_MAX = 4
};

enum class ETraceTypeQuery : uint8_t
{
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};

struct FLinearColor
{
	float                                              R;                                                        // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              G;                                                        // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              B;                                                        // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              A;                                                        // 0x000C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	inline FLinearColor()
		: R(0), G(0), B(0), A(0)
	{ }

	inline FLinearColor(float r, float g, float b, float a)
		: R(r),
		G(g),
		B(b),
		A(a)
	{ }

};

class UKismetSystemLibrary : public UObject
{
public:
	static inline void (*PrintStringOriginal)(UObject* Context, FFrame* Stack, void* Ret);

	static FString GetPathName(UObject* Object)
	{
		static auto GetPathNameFunction = FindObject<UFunction>("/Script/Engine.KismetSystemLibrary.GetPathName");
		static auto KismetSystemLibrary = FindObject("/Script/Engine.Default__KismetSystemLibrary");

		struct { UObject* Object; FString ReturnValue; } GetPathName_Params{ Object };

		KismetSystemLibrary->ProcessEvent(GetPathNameFunction, &GetPathName_Params);

		auto Ret = GetPathName_Params.ReturnValue;

		return Ret;
	}

	static void ExecuteConsoleCommand(UObject* WorldContextObject, const FString& Command, class APlayerController* SpecificPlayer)
	{
		static auto KismetSystemLibrary = FindObject("/Script/Engine.Default__KismetSystemLibrary");
		static auto fn = FindObject<UFunction>("/Script/Engine.KismetSystemLibrary.ExecuteConsoleCommand");

		struct {
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FString                                     Command;                                                  // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			class APlayerController* SpecificPlayer;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} UKismetSystemLibrary_ExecuteConsoleCommand_Params{WorldContextObject, Command, SpecificPlayer};

		KismetSystemLibrary->ProcessEvent(fn, &UKismetSystemLibrary_ExecuteConsoleCommand_Params);
	}

	static bool LineTraceSingle(UObject* WorldContextObject, FVector Start, FVector End, ETraceTypeQuery TraceChannel, bool bTraceComplex, 
		TArray<AActor*> ActorsToIgnore, EDrawDebugTrace DrawDebugType, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor,
		float DrawTime, FHitResult** OutHit)
	{
		static auto LineTraceSingleFn = FindObject<UFunction>("/Script/Engine.KismetSystemLibrary.LineTraceSingle");
		
		if (!LineTraceSingleFn)
			return false;

		struct UKismetSystemLibrary_LineTraceSingle_Params
		{
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     Start;                                                    // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     End;                                                      // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			ETraceTypeQuery                       TraceChannel;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			bool                                               bTraceComplex;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			TArray<AActor*>                              ActorsToIgnore;                                           // (ConstParm, Parm, OutParm, ZeroConstructor, ReferenceParm, NativeAccessSpecifierPublic)
			EDrawDebugTrace                       DrawDebugType;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FHitResult                                  OutHit;                                                   // (Parm, OutParm, IsPlainOldData, NoDestructor, ContainsInstancedReference, NativeAccessSpecifierPublic)
			bool                                               bIgnoreSelf;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FLinearColor                                TraceColor;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FLinearColor                                TraceHitColor;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			float                                              DrawTime;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			bool                                               ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		};

		static auto ReturnValueOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingle", "ReturnValue");
		static auto OutHitOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingle", "OutHit");
		static auto bIgnoreSelfOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingle", "bIgnoreSelf");

		static auto ParamsSize = LineTraceSingleFn->GetPropertiesSize();
		auto Params = Alloc(ParamsSize);

		((UKismetSystemLibrary_LineTraceSingle_Params*)Params)->WorldContextObject = WorldContextObject;
		((UKismetSystemLibrary_LineTraceSingle_Params*)Params)->Start = Start;
		((UKismetSystemLibrary_LineTraceSingle_Params*)Params)->End = End;
		((UKismetSystemLibrary_LineTraceSingle_Params*)Params)->TraceChannel = TraceChannel;
		((UKismetSystemLibrary_LineTraceSingle_Params*)Params)->bTraceComplex = bTraceComplex;
		*(bool*)(__int64(Params) + bIgnoreSelfOffset) = bIgnoreSelf;

		static auto KismetSystemLibrary = FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
		KismetSystemLibrary->ProcessEvent(LineTraceSingleFn, Params);

		if (OutHit)
			*OutHit = (FHitResult*)(__int64(Params) + OutHitOffset);

		bool ReturnValue = *(bool*)(__int64(Params) + ReturnValueOffset);

		// VirtualFree(Params, 0, MEM_RELEASE);

		return ReturnValue;
	}

	static bool LineTraceSingleByProfile(UObject* WorldContextObject, const FVector& Start, const FVector& End, FName ProfileName, bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace DrawDebugType, FHitResult** OutHit, bool bIgnoreSelf, const FLinearColor& TraceColor, 
		const FLinearColor& TraceHitColor, float DrawTime)
	{
		auto LineTraceSingleByProfileFn = FindObject<UFunction>(L"/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile");

		if (!LineTraceSingleByProfileFn)
			return false;

		struct UKismetSystemLibrary_LineTraceSingleByProfile_Params
		{
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     Start;                                                    // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     End;                                                      // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FName                                       ProfileName;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			bool                                               bTraceComplex;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			TArray<AActor*>                              ActorsToIgnore;                                           // (ConstParm, Parm, OutParm, ZeroConstructor, ReferenceParm, NativeAccessSpecifierPublic)
			EDrawDebugTrace                       DrawDebugType;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FHitResult                                  OutHit;                                                   // (Parm, OutParm, IsPlainOldData, NoDestructor, ContainsInstancedReference, NativeAccessSpecifierPublic)
			bool                                               bIgnoreSelf;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FLinearColor                                TraceColor;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FLinearColor                                TraceHitColor;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			float                                              DrawTime;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			bool                                               ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		};

		static auto ParamSize = LineTraceSingleByProfileFn->GetPropertiesSize();
		auto Params = Alloc(ParamSize);

		static auto WorldContextObjectOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "WorldContextObject");
		static auto StartOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "Start");
		static auto EndOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "End");
		static auto ProfileNameOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "ProfileName");
		static auto bTraceComplexOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "bTraceComplex");
		static auto ReturnValueOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "ReturnValue");
		static auto OutHitOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "OutHit");
		static auto bIgnoreSelfOffset = FindOffsetStruct("/Script/Engine.KismetSystemLibrary.LineTraceSingleByProfile", "bIgnoreSelf");

		*(UObject**)(__int64(Params) + WorldContextObjectOffset) = WorldContextObject;
		*(FVector*)(__int64(Params) + StartOffset) = Start;
		*(FVector*)(__int64(Params) + EndOffset) = End;
		*(FName*)(__int64(Params) + ProfileNameOffset) = ProfileName;
		*(bool*)(__int64(Params) + bTraceComplexOffset) = bTraceComplex;
		*(bool*)(__int64(Params) + bIgnoreSelfOffset) = bIgnoreSelf;

		static auto KismetSystemLibrary = FindObject("/Script/Engine.Default__KismetSystemLibrary");
		KismetSystemLibrary->ProcessEvent(LineTraceSingleByProfileFn, Params);

		if (OutHit)
			*OutHit = (FHitResult*)(__int64(Params) + OutHitOffset);

		return *(bool*)(__int64(Params) + ReturnValueOffset);
	}

	static void PrintStringHook(UObject* Context, FFrame* Stack, void* Ret);
};