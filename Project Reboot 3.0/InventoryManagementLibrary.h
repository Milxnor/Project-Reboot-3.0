#pragma once

#include "Object.h"
#include "Stack.h"

class UInventoryManagementLibrary : public UObject // UBlueprintFunctionLibrary
{
public:
	static inline void (*AddItemOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*AddItemsOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*GiveItemEntryToInventoryOwnerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*RemoveItemOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*RemoveItemsOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*SwapItemOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*SwapItemsOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	static void SwapItemsHook(UObject* Context, FFrame& Stack, void* Ret);
	static void SwapItemHook(UObject* Context, FFrame& Stack, void* Ret);
	static void RemoveItemsHook(UObject* Context, FFrame& Stack, void* Ret);
	static void RemoveItemHook(UObject* Context, FFrame& Stack, void* Ret);
	static void GiveItemEntryToInventoryOwnerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void AddItemsHook(UObject* Context, FFrame& Stack, void* Ret); // Return value changes 
	static void AddItemHook(UObject* Context, FFrame& Stack, void* Ret); // Return value changes 
};