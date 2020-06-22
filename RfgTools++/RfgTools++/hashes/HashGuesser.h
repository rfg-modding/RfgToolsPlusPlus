#pragma once
#include "common/Typedefs.h"
#include <optional>
#include <unordered_map>

class HashGuesser
{
public:
    static std::optional<string> GuessHashOriginString(u32 hash);

private:
    static void FillHashDictionary();

    static std::unordered_map<u32, string> hashDictionary_;
    static std::unordered_map<u32, string> hashDictionaryAlt_;
    static bool initialized_;
};