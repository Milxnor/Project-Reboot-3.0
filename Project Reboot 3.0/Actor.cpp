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

UClass* AActor::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.Actor");
	return Class;
}