#pragma once

#include "Core.h"

#include <string>
#include <vector>

namespace Core
{
    bool IsBase64(Byte c);

    std::string Base64Encode(Byte const *buf, uint32 bufLen);
    std::vector<Byte> Base64Decode(std::string const &encoded_string);
}

