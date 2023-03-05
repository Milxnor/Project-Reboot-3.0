#pragma once

#include "Object.h"
#include "NameTypes.h"

class UCurveTable : public UObject
{
public:
};

struct FCurveTableRowHandle
{
	UCurveTable* CurveTable;
	FName RowName; 
};