﻿#pragma once

/* TRCが何の略かは分からない (type / ??? / curse)*/
enum trc_curse_type {
	TRC_CURSED = 0x00000001L,
    TRC_HEAVY_CURSE = 0x00000002L,
    TRC_PERMA_CURSE = 0x00000004L,
    TRC_XXX1 = 0x00000008L,
    TRC_TY_CURSE = 0x00000010L,
    TRC_AGGRAVATE = 0x00000020L,
    TRC_DRAIN_EXP = 0x00000040L,
    TRC_SLOW_REGEN = 0x00000080L,
    TRC_ADD_L_CURSE = 0x00000100L,
    TRC_ADD_H_CURSE = 0x00000200L,
    TRC_CALL_ANIMAL = 0x00000400L,
    TRC_CALL_DEMON = 0x00000800L,
    TRC_CALL_DRAGON = 0x00001000L,
    TRC_COWARDICE = 0x00002000L,
    TRC_TELEPORT = 0x00004000L,
    TRC_LOW_MELEE = 0x00008000L,
    TRC_LOW_AC = 0x00010000L,
    TRC_LOW_MAGIC = 0x00020000L,
    TRC_FAST_DIGEST = 0x00040000L,
    TRC_DRAIN_HP = 0x00080000L,
    TRC_DRAIN_MANA = 0x00100000L,
    TRC_CALL_UNDEAD = 0x00200000L,
};

enum trc_special_type {
    TRC_TELEPORT_SELF = 0x00000001L,
    TRC_CHAINSWORD = 0x00000002L
};
