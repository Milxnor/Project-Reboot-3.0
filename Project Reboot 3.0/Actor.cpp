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

FVector AActor::GetActorLocation()
{
	static auto K2_GetActorLocationFn = FindObject<UFunction>("/Script/Engine.Actor.K2_GetActorLocation");
	FVector ret;
	this->ProcessEvent(K2_GetActorLocationFn, &ret);

	return ret;
}