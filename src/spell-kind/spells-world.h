﻿#pragma once

#include "system/angband.h"

void teleport_level(player_type *creature_ptr, MONSTER_IDX m_idx);
bool teleport_level_other(player_type *caster_ptr);
bool tele_town(player_type *caster_ptr);
void reserve_alter_reality(player_type *caster_ptr);
bool is_teleport_level_ineffective(player_type *caster_ptr, MONSTER_IDX idx);
