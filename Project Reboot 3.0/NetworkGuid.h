#pragma once

#include "inc.h"

class FNetworkGUID
{
public:
	uint32 Value;

	friend bool operator==(const FNetworkGUID& X, const FNetworkGUID& Y)
	{
		return (X.Value == Y.Value);
	}
};
