/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#include <cstdint>

class Utils
{
public:
    static uint32_t getChipId();
    static uint64_t generateDtuSerial();
};
