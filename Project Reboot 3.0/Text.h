#pragma once

#include "SharedPointer.h"
#include "inc.h"

struct ITextData
{

};

class FText
{
public:
	TSharedRef<ITextData, ESPMode::ThreadSafe> TextData;
	uint32 Flags;
};