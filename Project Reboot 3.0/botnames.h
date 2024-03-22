#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "discord.h"
#include "log.h"

static inline size_t WriteFunctionString(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

inline FString GetRandomBotNameGeneric() {
    std::string animal;
    std::string adjective;

    CURL* curl = curl_easy_init();
    if (curl) {
        // Get adjective
        std::string adjective_url = "http://random-word-form.herokuapp.com/random/adjective";
        curl_easy_setopt(curl, CURLOPT_URL, adjective_url.c_str());
        std::string adjective_response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFunctionString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &adjective_response);
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            adjective = adjective_response.substr(2, adjective_response.length() - 4);
        }

        // Get animal
        std::string animal_url = "http://random-word-form.herokuapp.com/random/animal";
        curl_easy_setopt(curl, CURLOPT_URL, animal_url.c_str());
        std::string animal_response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFunctionString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &animal_response);
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            animal = animal_response.substr(2, animal_response.length() - 4);
        }

        curl_easy_cleanup(curl);
    }

    srand(std::time(0) / rand());
    animal[0] = std::toupper(animal[0]);
    size_t spaceindex = animal.find(" ");
    if (spaceindex != std::string::npos) {
        animal.erase(spaceindex);
    }


    adjective[0] = std::toupper(adjective[0]);
    size_t dashindex = adjective.find("-");
    if (dashindex != std::string::npos) {
        adjective.erase(dashindex);
    }
    std::string ret = adjective + animal + std::to_string(rand() % 20000);
    return std::wstring(ret.begin(), ret.end()).c_str();
}


inline std::vector<FString> PlayerBotNames;

static inline void InitBotNames()
{
    PlayerBotNames.clear();

    PlayerBotNames.push_back(L"qwertyouriop");
    PlayerBotNames.push_back(L"willdey");
    PlayerBotNames.push_back(L"Soggs");
    PlayerBotNames.push_back(L"vxzty");
    PlayerBotNames.push_back(L"Milxnor");
    PlayerBotNames.push_back(L"max");
    PlayerBotNames.push_back(L"Callum");
    PlayerBotNames.push_back(L"Samicc");
    PlayerBotNames.push_back(L"AidasP");
    PlayerBotNames.push_back(L"danii");
    PlayerBotNames.push_back(L"sizzy");
    PlayerBotNames.push_back(L"penguin");
    PlayerBotNames.push_back(L"Jagger");
    PlayerBotNames.push_back(L"Jacobb");
    PlayerBotNames.push_back(L"Zulu");
    PlayerBotNames.push_back(L"kemo");
    PlayerBotNames.push_back(L"Ender");
    PlayerBotNames.push_back(L"Samuel");
    PlayerBotNames.push_back(L"Kyiro");
    PlayerBotNames.push_back(L"Ahava");
    PlayerBotNames.push_back(L"Spooky");
    PlayerBotNames.push_back(L"Akos");
    PlayerBotNames.push_back(L"ridecly");
    PlayerBotNames.push_back(L"cardurr");
    PlayerBotNames.push_back(L"android");
    PlayerBotNames.push_back(L"Sync");
    PlayerBotNames.push_back(L"GD");
    PlayerBotNames.push_back(L"Jeremy");
    PlayerBotNames.push_back(L"TeoVR");
    PlayerBotNames.push_back(L"CuteLess");
    PlayerBotNames.push_back(L"Trash Bot");
    PlayerBotNames.push_back(L"Lawin");
    PlayerBotNames.push_back(L"Adam");
    PlayerBotNames.push_back(L"Ruby");
    PlayerBotNames.push_back(L"Rythm");
    PlayerBotNames.push_back(L"Fexor");
    PlayerBotNames.push_back(L"Windermed");
    PlayerBotNames.push_back(L"BIGGINS");
    PlayerBotNames.push_back(L"Charles");
    PlayerBotNames.push_back(L"Custox");
    PlayerBotNames.push_back(L"Kaede");
    PlayerBotNames.push_back(L"lintu");
    PlayerBotNames.push_back(L"Baby");
    PlayerBotNames.push_back(L"Mineluke");
    PlayerBotNames.push_back(L"wiktorwiktor12");
    PlayerBotNames.push_back(L"solo");
    PlayerBotNames.push_back(L"HxD");
    PlayerBotNames.push_back(L"Noggo");
    PlayerBotNames.push_back(L"weyn");
    PlayerBotNames.push_back(L"ralz");
    PlayerBotNames.push_back(L"farex");
    PlayerBotNames.push_back(L"Nam");
    PlayerBotNames.push_back(L"AllyJax");
}
