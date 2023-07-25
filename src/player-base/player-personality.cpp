#include "player-base/player-personality.h"
#include "player/player-personality-types.h"
#include "player/player-personality.h"
#include "system/player-type-definition.h"

PlayerPersonality::PlayerPersonality(PlayerType *player_ptr)
    : player_ptr(player_ptr)
{
}

bool PlayerPersonality::equals(PlayerPersonalityType type) const
{
    return this->player_ptr->ppersonality == type;
}

const player_personality *PlayerPersonality::get_info() const
{
    return &personality_info[enum2i(this->player_ptr->ppersonality)];
}
