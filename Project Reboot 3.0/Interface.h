#pragma once

#include "Object.h"

/**
 * Base class for all interfaces
 *
 */

class UInterface : public UObject
{
	// DECLARE_CLASS_INTRINSIC(UInterface, UObject, CLASS_Interface | CLASS_Abstract, TEXT("/Script/CoreUObject"))
};

class IInterface
{
protected:

	virtual ~IInterface() {}

public:

	typedef UInterface UClassType;
};