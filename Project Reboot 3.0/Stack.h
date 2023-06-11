#pragma once

#include "OutputDevice.h"
#include "Class.h"

#define RESULT_DECL void*const RESULT_PARAM

struct FFrame : public FOutputDevice // https://github.com/EpicGames/UnrealEngine/blob/7acbae1c8d1736bb5a0da4f6ed21ccb237bc8851/Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h#L83
{
public:
	void** VFT; // 10

	// Variables.
	UFunction* Node; // 16
	UObject* Object; // 24 // 0x18
	uint8* Code; // 32 // 0x20
	uint8* Locals; // 40

	// MORE STUFF HERE

	void* MostRecentProperty; // 48
	uint8_t* MostRecentPropertyAddress; // 56

	void*& GetPropertyChainForCompiledIn()
	{
		static auto PropertyChainForCompiledInOffset = 0x80;
		return *(void**)(__int64(this) + PropertyChainForCompiledInOffset);
	}

	uint8_t*& GetMostRecentPropertyAddress()
	{
		auto off = (void*)(&((struct FFrame*)NULL)->MostRecentPropertyAddress);
		LOG_INFO(LogDev, "{}", off);
		return MostRecentPropertyAddress;
	}

	__forceinline void StepExplicitProperty(void* const Result, void* Property)
	{
		static void (*StepExplicitPropertyOriginal)(__int64 frame, void* const Result, void* Property) = decltype(StepExplicitPropertyOriginal)(Addresses::FrameStepExplicitProperty);
		StepExplicitPropertyOriginal(__int64(this), Result, Property);
	}

	__forceinline void Step(UObject* Context, RESULT_DECL)
	{
		static void (*StepOriginal)(__int64 frame, UObject* Context, RESULT_DECL) = decltype(StepOriginal)(Addresses::FrameStep);
		StepOriginal(__int64(this), Context, RESULT_PARAM);

		// int32 B = *Code++;
		// (GNatives[B])(Context, *this, RESULT_PARAM);
	}

	__forceinline void StepCompiledIn(void* const Result/*, const FFieldClass* ExpectedPropertyType*/, bool bPrint = false) // https://github.com/EpicGames/UnrealEngine/blob/cdaec5b33ea5d332e51eee4e4866495c90442122/Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h#L444
	{
		if (Code)
		{
			Step(Object, Result);
		}
		else
		{
			// LOG_INFO(LogDev, "UNIMPLENTED!");
			/* checkSlow(ExpectedPropertyType && ExpectedPropertyType->IsChildOf(FProperty::StaticClass()));
			checkSlow(PropertyChainForCompiledIn && PropertyChainForCompiledIn->IsA(ExpectedPropertyType));
			FProperty* Property = (FProperty*)PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = Property->Next;
			StepExplicitProperty(Result, Property); */

			if (bPrint)
				LOG_INFO(LogDev, "No code!");

			void* Property = GetPropertyChainForCompiledIn();
			GetPropertyChainForCompiledIn() = GetNext(Property);
			StepExplicitProperty(Result, Property);
		}
	}

	template</* class TProperty, */ typename TNativeType>
	__forceinline TNativeType& StepCompiledInRef(void* const TemporaryBuffer)
	{
		GetMostRecentPropertyAddress() = nullptr;
		// GetMostRecentPropertyContainer() = nullptr; // added in ue5.1

		if (Code)
		{
			Step(Object, TemporaryBuffer);
		}
		else
		{
			LOG_INFO(LogDev, "UNIMPLENTED2!");

			/* checkSlow(CastField<TProperty>(PropertyChainForCompiledIn) && CastField<FProperty>(PropertyChainForCompiledIn));
			TProperty* Property = (TProperty*)PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = Property->Next;
			StepExplicitProperty(TemporaryBuffer, Property); */
		}

		return (GetMostRecentPropertyAddress() != NULL) ? *(TNativeType*)(GetMostRecentPropertyAddress()) : *(TNativeType*)TemporaryBuffer;
	}
};