#include "KismetSystemLibrary.h"

#include "gui.h"

void UKismetSystemLibrary::PrintStringHook(UObject* Context, FFrame* Stack, void* Ret)
{
	UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FString                                     inString;                                                 // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bPrintToScreen;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bPrintToLog;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FLinearColor                                TextColor;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              Duration;

	Stack->StepCompiledIn(&WorldContextObject);
	Stack->StepCompiledIn(&inString);
	Stack->StepCompiledIn(&bPrintToScreen);
	Stack->StepCompiledIn(&bPrintToLog);
	Stack->StepCompiledIn(&TextColor);
	Stack->StepCompiledIn(&Duration);

	if (bEngineDebugLogs)
		LOG_INFO(LogDev, "GameLog: {}", inString.ToString());

	return PrintStringOriginal(Context, Stack, Ret);
}