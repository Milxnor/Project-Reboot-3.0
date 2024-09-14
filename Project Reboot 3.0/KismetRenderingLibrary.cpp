#include "KismetRenderingLibrary.h"

#include "reboot.h"

void UKismetRenderingLibrary::ExportTexture2D(UObject* WorldContextObject, class UTexture2D* Texture, const FString& FilePath, const FString& Filename)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.KismetRenderingLibrary.ExportTexture2D");

	struct { UObject* WorldContextObject; class UTexture2D* Texture; FString FilePath; FString Filename; } params{ WorldContextObject, Texture, FilePath, Filename };

	static auto DefaultObject = StaticClass();
	DefaultObject->ProcessEvent(fn, &params);
}

UClass* UKismetRenderingLibrary::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.KismetRenderingLibrary");
	return Class;
}