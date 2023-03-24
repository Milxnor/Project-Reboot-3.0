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

	void* MostRecentProperty;
	uint8_t* MostRecentPropertyAddress;

	uint8_t*& GetMostRecentPropertyAddress()
	{
		auto off = (void*)(&((struct FFrame*)NULL)->MostRecentPropertyAddress);
		LOG_INFO(LogDev, "{}", off);
		return MostRecentPropertyAddress;
		static auto MostRecentPropertyAddressOffset = 56;
		return *(uint8_t**)(__int64(this) + MostRecentPropertyAddressOffset);
	}

	void Step(UObject* Context, RESULT_DECL)
	{
		static void (*StepOriginal)(__int64 frame, UObject* Context, RESULT_DECL) = decltype(StepOriginal)(Addresses::FrameStep);
		StepOriginal(__int64(this), Context, RESULT_PARAM);

		// int32 B = *Code++;
		// (GNatives[B])(Context, *this, RESULT_PARAM);
	}

	__forceinline void StepCompiledIn(void* Result/*, const FFieldClass* ExpectedPropertyType*/) // https://github.com/EpicGames/UnrealEngine/blob/cdaec5b33ea5d332e51eee4e4866495c90442122/Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h#L444
	{
		if (Code)
		{
			Step(Object, Result);
		}
		else
		{
			LOG_INFO(LogDev, "UNIMPLENTED!");
			/* checkSlow(ExpectedPropertyType && ExpectedPropertyType->IsChildOf(FProperty::StaticClass()));
			checkSlow(PropertyChainForCompiledIn && PropertyChainForCompiledIn->IsA(ExpectedPropertyType));
			FProperty* Property = (FProperty*)PropertyChainForCompiledIn;
			PropertyChainForCompiledIn = Property->Next;
			StepExplicitProperty(Result, Property); */
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