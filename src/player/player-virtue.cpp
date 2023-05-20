#include "player/player-virtue.h"
#include "avatar/avatar.h"
#include "system/player-type-definition.h"
#include <algorithm>

PlayerVirtue::PlayerVirtue(PlayerType *player_ptr)
    : player_ptr(player_ptr)
{
}

bool PlayerVirtue::has(Virtue virtue) const
{
    return this->get(virtue).has_value();
}

/*!
 * @brief 指定した徳の値を得る
 *
 * @param virtue 値を得る徳の種別
 * @return 指定した徳の値。指定した徳をプレイヤーが所持していなければstd::nullopt。
 */
std::optional<short> PlayerVirtue::get(Virtue virtue) const
{
    const auto &types = this->player_ptr->vir_types;
    const auto it = std::find(std::begin(types), std::end(types), virtue);
    if (it == std::end(types)) {
        return std::nullopt;
    }

    const auto index = std::distance(std::begin(types), it);
    return this->player_ptr->virtues[index];
}
