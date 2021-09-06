#pragma once

#include "object-enchant/tr-types.h"
#include "util/flag-group.h"

/*! オブジェクトの特性フラグtr_typeの集合を表すクラス */
using TrFlags = FlagGroup<tr_type, TR_FLAG_MAX>;

// clang-format off

/*! 能力値(STR,INT,WIS,DEX,CON,CHR)を増減させる特性フラグがONになっているTrFlags定数オブジェクト */
inline const TrFlags TR_STATUS_FLAG_MASK{
    TR_STATUS_LIST.begin(), TR_STATUS_LIST.end()
};

/*! 能力値(STR,INT,WIS,DEX,CON,CHR)を維持する特性フラグがONになっているTrFlags定数オブジェクト */
inline const TrFlags TR_SUST_STATUS_FLAG_MASK{
    TR_SUST_STATUS_LIST.begin(), TR_SUST_STATUS_LIST.end()
};

/*! 基本属性(酸、電撃、火炎、冷気)の耐性フラグがONになっているTrFlags定数オブジェクト */
inline const TrFlags TR_RES_ELEMENT_FLAG_MASK{
    TR_RES_ELEMENT_LIST.begin(), TR_RES_ELEMENT_LIST.end()
};

/*! 基本属性(酸、電撃、火炎、冷気)の免疫フラグがONになっているTrFlags定数オブジェクト */
inline const TrFlags TR_IM_ELEMENT_FLAG_MASK{
    TR_IM_ELEMENT_LIST.begin(), TR_IM_ELEMENT_LIST.end()
};

/*! 基本属性(酸、電撃、火炎、冷気)の耐破壊フラグがONになっているTrFlags定数オブジェクト */
inline const TrFlags TR_IGNORE_ELEMENT_FLAG_MASK{
    TR_IGNORE_ELEMENT_LIST.begin(), TR_IGNORE_ELEMENT_LIST.end()
};

/*! pvalを増減させる特性フラグがONになっているFlagGroup定数オブジェクト */
inline const TrFlags TR_PVAL_FLAG_MASK =
    TrFlags(TR_STATUS_FLAG_MASK).set({
        TR_MAGIC_MASTERY,
        TR_STEALTH,
        TR_SEARCH,
        TR_INFRA,
        TR_TUNNEL,
        TR_SPEED,
        TR_BLOWS,
    });

// clang-format on
