#include "Controller.h"

#include "reboot.h"

AActor* AController::GetViewTarget()
{
	static auto GetViewTargetFn = FindObject<UFunction>("/Script/Engine.Controller.GetViewTarget");
	AActor* ViewTarget = nullptr;
	this->ProcessEvent(GetViewTargetFn, &ViewTarget);
	return ViewTarget;
}

void AController::Possess(class APawn* Pawn)
{
	auto PossessFn = FindFunction("Possess");
	this->ProcessEvent(PossessFn, &Pawn);
}