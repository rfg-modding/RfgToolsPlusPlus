//License for the hash functions used in this file below. Translated from C#. Origin: https://github.com/gibbed/Gibbed.Volition

/* Copyright (c) 2017 Rick (rick 'at' gibbed 'dot' us)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 */

//Note: HashVolitionCRCAlt() was not part of the original source

#pragma once
#include "common/Typedefs.h"
#include "common/String.h"

namespace Hash
{
    u32 HashVolitionCRC(std::string_view input, u32 hash);
    u32 HashVolitionCRCAlt(std::string_view input, u32 hash);
    u32 HashVolition(std::string_view input);
}