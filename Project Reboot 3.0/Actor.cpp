#include "Actor.h"

#include "Transform.h"

#include "reboot.h"

FTransform AActor::GetTransform()
{
	FTransform Ret;
	static auto fn = FindObject<UFunction>(L"/Script/Engine.Actor.GetTransform");
	this->ProcessEvent(fn, &Ret);
	return Ret;
}

AActor* AActor::GetOwner()
{
	static auto GetOwnerFunction = FindObject<UFunction>("/Script/Engine.Actor.GetOwner");

	AActor* Owner = nullptr;
	this->ProcessEvent(GetOwnerFunction, &Owner);

	return Owner;
}

void AActor::K2_DestroyActor()
{
	static auto DestroyActorFn = FindObject<UFunction>("/Script/Engine.Actor.K2_DestroyActor");

	this->ProcessEvent(DestroyActorFn);
}

UActorComponent* AActor::GetComponentByClass(class UClass* ComponentClass)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.Actor.GetComponentByClass");
	struct
	{
		class UClass* ComponentClass;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UActorComponent* ReturnValue;                                              // (ExportObject, Parm, OutParm, ZeroConstructor, ReturnParm, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AActor_GetComponentByClass_Params{ComponentClass};

	this->ProcessEvent(fn, &AActor_GetComponentByClass_Params);

	return AActor_GetComponentByClass_Params.ReturnValue;
}

float AActor::GetDistanceTo(AActor* OtherActor)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.Actor.GetDistanceTo");

	struct { AActor* OtherActor; float ReturnValue; } AActor_GetDistanceTo_Params{OtherActor};

	this->ProcessEvent(fn, &AActor_GetDistanceTo_Params);

	return AActor_GetDistanceTo_Params.ReturnValue;
}

FVector AActor::GetActorLocation()
{
	static auto K2_GetActorLocationFn = FindObject<UFunction>("/Script/Engine.Actor.K2_GetActorLocation");
	FVector ret;
	this->ProcessEvent(K2_GetActorLocationFn, &ret);

	return ret;
}

FVector AActor::GetActorRightVector()
{
	static auto GetActorRightVectorFn = FindObject<UFunction>("/Script/Engine.Actor.GetActorRightVector");
	FVector ret;
	this->ProcessEvent(GetActorRightVectorFn, &ret);

	return ret;
}

FRotator AActor::GetActorRotation()
{
	static auto K2_GetActorRotationFn = FindObject<UFunction>(L"/Script/Engine.Actor.K2_GetActorRotation");
	FRotator ret;
	this->ProcessEvent(K2_GetActorRotationFn, &ret);

	return ret;
}

void AActor::FlushNetDormancy()
{
	static auto fn = FindObject<UFunction>("/Script/Engine.Actor.FlushNetDormancy");
	this->ProcessEvent(fn);
}

bool AActor::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.Actor.K2_TeleportTo");
	struct
	{
		struct FVector                                     DestLocation;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		struct FRotator                                    DestRotation;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
		bool                                               ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AActor_K2_TeleportTo_Params{DestLocation, DestRotation};
	this->ProcessEvent(fn, &AActor_K2_TeleportTo_Params);
	return AActor_K2_TeleportTo_Params.ReturnValue;
}

bool AActor::IsActorBeingDestroyed()
{
	static auto bActorIsBeingDestroyedOffset = GetOffset("bActorIsBeingDestroyed");
	static auto bActorIsBeingDestroyedFieldMask = GetFieldMask(GetProperty("bActorIsBeingDestroyed"));
	return ReadBitfieldValue(bActorIsBeingDestroyedOffset, bActorIsBeingDestroyedFieldMask);
}

bool AActor::IsNetStartup()
{
	static auto bNetStartupOffset = GetOffset("bNetStartup");
	static auto bNetStartupFieldMask = GetFieldMask(GetProperty("bNetStartup"));
	return ReadBitfieldValue(bNetStartupOffset, bNetStartupFieldMask);
}

void AActor::SetOwner(AActor* Owner)
{
	static auto SetOwnerFn = FindObject<UFunction>("/Script/Engine.Actor.SetOwner");
	this->ProcessEvent(SetOwnerFn, &Owner);
}

void AActor::ForceNetUpdate()
{
	static auto ForceNetUpdateFn = FindObject<UFunction>("/Script/Engine.Actor.ForceNetUpdate");
	this->ProcessEvent(ForceNetUpdateFn);
}

bool AActor::IsAlwaysRelevant()
{
	static auto bAlwaysRelevantOffset = GetOffset("bAlwaysRelevant");
	static auto bAlwaysRelevantFieldMask = GetFieldMask(GetProperty("bAlwaysRelevant"));
	return ReadBitfieldValue(bAlwaysRelevantOffset, bAlwaysRelevantFieldMask);
}

bool AActor::UsesOwnerRelevancy()
{
	static auto bNetUseOwnerRelevancyOffset = GetOffset("bNetUseOwnerRelevancy");
	static auto bNetUseOwnerRelevancyFieldMask = GetFieldMask(GetProperty("bNetUseOwnerRelevancy"));
	return ReadBitfieldValue(bNetUseOwnerRelevancyOffset, bNetUseOwnerRelevancyFieldMask);
}

bool AActor::IsOnlyRelevantToOwner()
{
	static auto bOnlyRelevantToOwnerOffset = GetOffset("bOnlyRelevantToOwner");
	static auto bOnlyRelevantToOwnerFieldMask = GetFieldMask(GetProperty("bOnlyRelevantToOwner"));
	return ReadBitfieldValue(bOnlyRelevantToOwnerOffset, bOnlyRelevantToOwnerFieldMask);
}

bool AActor::CanBeDamaged()
{
	static auto bCanBeDamagedOffset = GetOffset("bCanBeDamaged");
	static auto bCanBeDamagedFieldMask = GetFieldMask(GetProperty("bOnlyRelevantToOwner"));
	return ReadBitfieldValue(bCanBeDamagedOffset, bCanBeDamagedFieldMask);
}

void AActor::SetCanBeDamaged(bool NewValue)
{
	static auto bCanBeDamagedOffset = GetOffset("bCanBeDamaged");
	static auto bCanBeDamagedFieldMask = GetFieldMask(GetProperty("bOnlyRelevantToOwner"));
	SetBitfieldValue(bCanBeDamagedOffset, bCanBeDamagedFieldMask, NewValue);
}

UClass* AActor::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.Actor");
	return Class;
}