#include "FortDecoItemDefinition.h"

static UClass* StaticClass()
{
	static auto ptr = FindObject<UClass>("/Script/FortniteGame.FortDecoItemDefinition");
	return ptr;
}