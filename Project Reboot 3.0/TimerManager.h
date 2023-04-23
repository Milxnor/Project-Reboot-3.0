#pragma once

#include "EngineTypes.h"
#include "Function.h"
#include "DelegateCombinations.h"

DECLARE_DELEGATE(FTimerDelegate);

struct FTimerUnifiedDelegate
{
	/** Holds the delegate to call. */
	FTimerDelegate FuncDelegate;
	/** Holds the dynamic delegate to call. */
	FTimerDynamicDelegate FuncDynDelegate;
	/** Holds the TFunction callback to call. */
	TFunction<void(void)> FuncCallback;

	FTimerUnifiedDelegate() {};
	FTimerUnifiedDelegate(FTimerDelegate const& D) : FuncDelegate(D) {};
};

class FTimerManager // : public FNoncopyable
{
public:
	FORCEINLINE void SetTimer(FTimerHandle& InOutHandle, FTimerDelegate const& InDelegate, float InRate, bool InbLoop, float InFirstDelay = -1.f)
	{
		static void (*InternalSetTimerOriginal)(__int64 TimerManager, FTimerHandle& InOutHandle, FTimerUnifiedDelegate&& InDelegate, float InRate, bool InbLoop, float InFirstDelay) = 
			decltype(InternalSetTimerOriginal)(Addresses::SetTimer);

		InternalSetTimerOriginal(__int64(this), InOutHandle, FTimerUnifiedDelegate(InDelegate), InRate, InbLoop, InFirstDelay);
	}
};