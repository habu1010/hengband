/*!
 * @brief プレイヤーの時限効果を表すオブジェクト群を保持する
 * @date 2022/08/05
 * @author Hourier
 */

#include "timed-effect/timed-effects.h"
#include "timed-effect/player-acceleration.h"
#include "timed-effect/player-blindness.h"
#include "timed-effect/player-confusion.h"
#include "timed-effect/player-cut.h"
#include "timed-effect/player-deceleration.h"
#include "timed-effect/player-fear.h"
#include "timed-effect/player-hallucination.h"
#include "timed-effect/player-paralysis.h"
#include "timed-effect/player-poison.h"
#include "timed-effect/player-stun.h"

struct TimedEffects::Impl {
    PlayerBlindness player_blindness{};
    PlayerConfusion player_confusion{};
    PlayerCut player_cut{};
    PlayerFear player_fear{};
    PlayerHallucination player_hallucination{};
    PlayerParalysis player_paralysis{};
    PlayerStun player_stun{};
    PlayerAcceleration player_acceleration{};
    PlayerDeceleration player_deceleration{};
    PlayerPoison player_poison{};
};

TimedEffects::TimedEffects()
    : pimpl(std::make_unique<Impl>())
{
}

TimedEffects::~TimedEffects() = default;

PlayerBlindness &TimedEffects::blindness()
{
    return pimpl->player_blindness;
}

const PlayerBlindness &TimedEffects::blindness() const
{
    return pimpl->player_blindness;
}

PlayerConfusion &TimedEffects::confusion()
{
    return pimpl->player_confusion;
}

const PlayerConfusion &TimedEffects::confusion() const
{
    return pimpl->player_confusion;
}

PlayerCut &TimedEffects::cut()
{
    return pimpl->player_cut;
}

const PlayerCut &TimedEffects::cut() const
{
    return pimpl->player_cut;
}

PlayerFear &TimedEffects::fear()
{
    return pimpl->player_fear;
}

const PlayerFear &TimedEffects::fear() const
{
    return pimpl->player_fear;
}

PlayerHallucination &TimedEffects::hallucination()
{
    return pimpl->player_hallucination;
}

const PlayerHallucination &TimedEffects::hallucination() const
{
    return pimpl->player_hallucination;
}

PlayerParalysis &TimedEffects::paralysis()
{
    return pimpl->player_paralysis;
}

const PlayerParalysis &TimedEffects::paralysis() const
{
    return pimpl->player_paralysis;
}

PlayerStun &TimedEffects::stun()
{
    return pimpl->player_stun;
}

const PlayerStun &TimedEffects::stun() const
{
    return pimpl->player_stun;
}

PlayerAcceleration &TimedEffects::acceleration()
{
    return pimpl->player_acceleration;
}

const PlayerAcceleration &TimedEffects::acceleration() const
{
    return pimpl->player_acceleration;
}

PlayerDeceleration &TimedEffects::deceleration()
{
    return pimpl->player_deceleration;
}

const PlayerDeceleration &TimedEffects::deceleration() const
{
    return pimpl->player_deceleration;
}

PlayerPoison &TimedEffects::poison()
{
    return pimpl->player_poison;
}

const PlayerPoison &TimedEffects::poison() const
{
    return pimpl->player_poison;
}
