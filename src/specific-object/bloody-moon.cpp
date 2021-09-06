#include "specific-object/bloody-moon.h"
#include "artifact/fixed-art-types.h"
#include "core/player-update-types.h"
#include "object-enchant/object-boost.h"
#include "object-enchant/tr-types.h"
#include "racial/racial-android.h"
#include "system/artifact-type-definition.h"
#include "system/object-type-definition.h"
#include "system/player-type-definition.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"

namespace {
/**
 * @brief 固定アーティファクト『ブラッディムーン』の発動で付く可能性のある攻撃系特性フラグのリスト。確率は等確率。
 * KILL_EVILは強力すぎるため除外されていると思われる。
 * @todo 後に善良スレイが追加されているが、加える必要は無いか？
 */
constexpr std::array<tr_type, 26> bloody_moon_slaying_list = {
    TR_SLAY_ANIMAL,
    TR_SLAY_EVIL,
    TR_SLAY_UNDEAD,
    TR_SLAY_DEMON,
    TR_SLAY_ORC,
    TR_SLAY_TROLL,
    TR_SLAY_GIANT,
    TR_SLAY_DRAGON,
    TR_SLAY_HUMAN,
    TR_KILL_ANIMAL,
    TR_KILL_UNDEAD,
    TR_KILL_DEMON,
    TR_KILL_ORC,
    TR_KILL_TROLL,
    TR_KILL_GIANT,
    TR_KILL_DRAGON,
    TR_KILL_HUMAN,
    TR_CHAOTIC,
    TR_VAMPIRIC,
    TR_VORPAL,
    TR_EARTHQUAKE,
    TR_BRAND_POIS,
    TR_BRAND_ACID,
    TR_BRAND_ELEC,
    TR_BRAND_FIRE,
    TR_BRAND_COLD,
};

/**
 * @brief 固定アーティファクト『ブラッディムーン』の発動で付く可能性のあるpval能力系特性フラグのリスト。確率は等確率。
 */
constexpr std::array<tr_type, 11> bloody_moon_pval_list = {
    TR_STR,
    TR_INT,
    TR_WIS,
    TR_DEX,
    TR_CON,
    TR_CHR,
    TR_STEALTH,
    TR_SEARCH,
    TR_INFRA,
    TR_TUNNEL,
    TR_SPEED,
};
}

/*!
 * @brief 固定アーティファクト『ブラッディムーン』の特性を変更する。
 * @details スレイ2d2種、及びone_resistance()による耐性1d2種、pval2種を得る。
 * @param o_ptr 対象のオブジェクト構造体 (ブラッディムーン)のポインタ
 */
void get_bloody_moon_flags(object_type *o_ptr)
{
    o_ptr->art_flags = a_info[ART_BLOOD].flags;

    for (auto count = damroll(2, 2); count > 0; count--) {
        auto flag = rand_choice(bloody_moon_slaying_list);
        o_ptr->art_flags.set(flag);
    }

    for (auto count = randint1(2); count > 0; count--) {
        one_resistance(o_ptr);
    }

    for (auto i = 0; i < 2; i++) {
        auto flag = rand_choice(bloody_moon_pval_list);
        o_ptr->art_flags.set(flag);
    }
}

/*!
 * @brief Let's dance a RONDO!!
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param o_ptr ブラッディ・ムーンへの参照ポインタ
 * @return オブジェクト情報に異常がない限りTRUE
 */
bool activate_bloody_moon(player_type *player_ptr, object_type *o_ptr)
{
    if (o_ptr->name1 != ART_BLOOD)
        return false;

    msg_print(_("鎌が明るく輝いた...", "Your scythe glows brightly!"));
    get_bloody_moon_flags(o_ptr);
    if (player_ptr->prace == player_race_type::ANDROID)
        calc_android_exp(player_ptr);

    player_ptr->update |= PU_BONUS | PU_HP;
    return true;
}
