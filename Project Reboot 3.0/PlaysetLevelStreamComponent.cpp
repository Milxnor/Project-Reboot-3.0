#include "PlaysetLevelStreamComponent.h"

void UPlaysetLevelStreamComponent::SetPlaysetHook(UPlaysetLevelStreamComponent* Component, UFortPlaysetItemDefinition* NewPlayset)
{
	SetPlaysetOriginal(Component, NewPlayset);

	auto Volume = Cast<AFortVolume>(Component->GetOwner());

	LOG_INFO(LogDev, "Volume: {}", __int64(Volume));

	if (!Volume || !Volume->HasAuthority())
		return;
}