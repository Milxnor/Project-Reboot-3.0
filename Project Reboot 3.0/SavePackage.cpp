#include "Package.h"

#include "reboot.h"

UClass* UPackage::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/CoreUObject.Package");
	return Class;
}