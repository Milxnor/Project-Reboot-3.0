// I don't even know what this is.

#pragma once

#include "inc.h"
#include <unordered_map>
#include "Object.h"

enum class Severity
{
	LOW, // Could be commonly false, most likely due to lag.
	MEDIUM, // Extreme rare cases this is a false.
	HIGH // Not possible to be false, only happens with modification of client.
};

class AnticheatComponent
{
public:
	UObject* Owner;
	int AmountOfLowWarnings;
	int AmountOfMediumWarnings;
	int AmountOfHighWarnings;

	void Kick()
	{

	}

	bool IsCheater()
	{
		if (AmountOfHighWarnings >= 3)
		{
			Kick();
			return true;
		}

		return false;
	}

	bool AddAndCheck(Severity severity)
	{
		if (severity == Severity::LOW)
			AmountOfLowWarnings++;
		if (severity == Severity::MEDIUM)
			AmountOfMediumWarnings++;
		if (severity == Severity::LOW)
			AmountOfHighWarnings++;

		return IsCheater();
	}
};

// std::unordered_map<UObject*, AnticheatComponent> AnticheatComponents;