#include "BuildingSMActor.h"

UClass* ABuildingSMActor::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.BuildingSMActor");
	return Class;
}