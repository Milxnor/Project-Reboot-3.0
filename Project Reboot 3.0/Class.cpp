#pragma once

#include "Class.h"
#include "reboot.h"

UObject* UClass::CreateDefaultObject()
{
    static std::unordered_map<std::string, UObject*> defaultAbilities; // normal class name, default ability.

    auto name = this->GetFullName();

    auto defaultafqaf = defaultAbilities.find(name);

    UObject* DefaultObject = nullptr;

    if (defaultafqaf != defaultAbilities.end())
    {
        DefaultObject = defaultafqaf->second;
    }
    else
    {
        // skunked class to default
        auto ending = name.substr(name.find_last_of(".") + 1);
        auto path = name.substr(0, name.find_last_of(".") + 1);

        path = path.substr(path.find_first_of(" ") + 1);

        auto DefaultAbilityName = std::format("{0}Default__{1}", path, ending);

        // std::cout << "DefaultAbilityName: " << DefaultAbilityName << '\n';

        DefaultObject = FindObject(DefaultAbilityName);
        defaultAbilities.emplace(name, DefaultObject);
    }

	return DefaultObject;
}

int UClass::GetPropertiesSize()
{
	return *(int*)(__int64(this) + Offsets::PropertiesSize);
}