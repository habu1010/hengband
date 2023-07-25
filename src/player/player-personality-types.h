#pragma once

#include "util/enum-converter.h"

enum class PlayerPersonalityType {
    ORDINARY = 0,
    MIGHTY = 1,
    SHREWD = 2,
    PIOUS = 3,
    NIMBLE = 4,
    FEARLESS = 5,
    COMBAT = 6,
    LAZY = 7,
    SEXY = 8,
    LUCKY = 9,
    PATIENT = 10,
    MUNCHKIN = 11,
    CHARGEMAN = 12,
    MAX,
};

constexpr auto MAX_PERSONALITIES = enum2i(PlayerPersonalityType::MAX);
