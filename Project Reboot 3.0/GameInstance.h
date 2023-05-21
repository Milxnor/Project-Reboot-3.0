#pragma once

#include "Object.h"

#include "TimerManager.h"

class UGameInstance : public UObject
{
public:
    inline FTimerManager& GetTimerManager() const 
    {
        static auto TimerManagerOffset = 0x90;
        return **(FTimerManager**)(__int64(TimerManagerOffset));
    }
};