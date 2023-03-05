#include "KismetMathLibrary.h"

#include "reboot.h"

float UKismetMathLibrary::RandomFloatInRange(float min, float max)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.KismetMathLibrary.RandomFloatInRange");

	struct { float min; float max; float ret; } params{min, max};

	static auto DefaultObject = StaticClass();
	DefaultObject->ProcessEvent(fn, &params);

	return params.ret;
}

UClass* UKismetMathLibrary::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/Engine.KismetMathLibrary");
	return Class;
}