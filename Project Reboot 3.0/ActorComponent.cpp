#include "ActorComponent.h"

#include "reboot.h"

AActor* UActorComponent::GetOwner()
{
	auto GetOwnerFn = FindObject<UFunction>(L"/Script/Engine.ActorComponent.GetOwner");
	AActor* Owner;
	this->ProcessEvent(GetOwnerFn, &Owner);
	return Owner;
}