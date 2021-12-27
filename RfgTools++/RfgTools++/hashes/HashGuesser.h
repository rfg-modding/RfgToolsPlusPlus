#pragma once
#include "common/Typedefs.h"
#include "common/String.h"
#include <unordered_map>
#include <optional>
#include <mutex>

class HashGuesser
{
public:
    static std::optional<string> GuessHashOriginString(u32 hash);

private:
    static void FillHashDictionary();

    static std::unordered_map<u32, string> hashDictionary_;
    static std::unordered_map<u32, string> hashDictionaryAlt_;
    static bool initialized_;
    static std::mutex mutex_; //Used to avoid multiple threads calling FillHashDictionary at once if it's the first use
};