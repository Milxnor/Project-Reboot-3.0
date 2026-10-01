#pragma once

#define CURL_STATICLIB

#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>

// Small helper to escape JSON strings correctly
inline std::string DiscordJsonEscape(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (char c : input)
    {
        switch (c)
        {
        case '\"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:
            if (static_cast<unsigned char>(c) < 0x20)
                out += ' ';
            else
                out += c;
        }
    }
    return out;
}

// Helper to create a Discord-compatible ISO-8601 timestamp (UTC)
inline std::string DiscordCurrentTimestampUTC()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);

    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &t);
#else
    gmtime_r(&t, &tm_utc);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S.000Z");
    return oss.str();
}

class DiscordWebhook {
public:
    DiscordWebhook(const char* webhook_url)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, webhook_url);

            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        else {
            std::cerr << "Error: curl_easy_init() returned NULL pointer" << '\n';
        }
    }

    ~DiscordWebhook()
    {
        if (headers)
            curl_slist_free_all(headers);
        if (curl)
            curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    bool handleCode(CURLcode res)
    {
        if (res != CURLE_OK)
        {
            std::cerr << "Discord webhook request failed: "
                << curl_easy_strerror(res) << "\n";
            return false;
        }
        return true;
    }

    inline bool send_message(const std::string& message)
    {
        std::string json = std::string("{\"content\": \"") +
            DiscordJsonEscape(message) + "\"}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        return handleCode(curl_easy_perform(curl));
    }

    inline bool send_embedjson(const std::string& ajson)
    {
        std::string json;
        if (ajson.find("\"embeds\"") != std::string::npos)
            json = ajson;
        else
            json = "{\"embeds\": " + ajson + "}";

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
        return handleCode(curl_easy_perform(curl));
    }

    inline bool send_embed(const std::string& title,
        const std::string& description,
        int color = 0x5865F2,
        const std::string& headerText = "",
        const std::string& footerText = "",
        const std::string& footerIconUrl = "",
        const std::string& thumbnailUrl = "",
        bool includeTimestamp = true)
    {
        std::string json = "{ \"embeds\": [ {";

        // Title
        json += "\"title\": \"" + DiscordJsonEscape(title) + "\"";

        // Description
        json += ", \"description\": \"" + DiscordJsonEscape(description) + "\"";

        // Color
        json += ", \"color\": " + std::to_string(color);

        // Optional thumbnail (image on the right)
        if (!thumbnailUrl.empty())
        {
            json += ", \"thumbnail\": { \"url\": \"" +
                DiscordJsonEscape(thumbnailUrl) + "\" }";
        }

        // Optional header
        if (!headerText.empty())
        {
            json += ", \"header\": { \"text\": \"" +
                DiscordJsonEscape(headerText) + "\" }";
        }

        // Optional footer (text + small icon)
        if (!footerText.empty())
        {
            json += ", \"footer\": { \"text\": \"" +
                DiscordJsonEscape(footerText) + "\"";

            if (!footerIconUrl.empty())
            {
                json += ", \"icon_url\": \"" +
                    DiscordJsonEscape(footerIconUrl) + "\"";
            }

            json += " }";
        }

        // Optional timestamp
        if (includeTimestamp)
        {
            json += ", \"timestamp\": \"" + DiscordCurrentTimestampUTC() + "\"";
        }

        json += "} ] }";

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
        return handleCode(curl_easy_perform(curl));
    }

private:
    CURL* curl = nullptr;
    curl_slist* headers = nullptr;
};

// ---------------- Configuration & global instance ----------------

namespace Information
{
    // Put your webhook URL here
    static std::string UptimeWebHook = "";

    // Images used by embeds
    static std::string EmbedThumbnailUrl = "https://i.imgur.com/2RImwlb.png";      // big image on the right
    static std::string EmbedFooterIconUrl = "https://i.imgur.com/2RImwlb.png";    // small icon in footer
}

static DiscordWebhook UptimeWebHook(Information::UptimeWebHook.c_str());

// Example usage:
// 
// UptimeWebHookClient.send_embed(
//     "title",                                 // title
//     "description",                           // description
//     0x57F287,                                // green
//     "Reboot Server",                         // header text
//     "Reboot Server",                         // footer text
//     Information::EmbedFooterIconUrl,         // footer icon
//     Information::EmbedThumbnailUrl,          // thumbnail (right side)
//     true                                     // include timestamp
// );
