#pragma once

#include <memory>

class PlayerBlindness;
class PlayerConfusion;
class PlayerCut;
class PlayerFear;
class PlayerHallucination;
class PlayerParalysis;
class PlayerStun;
class PlayerAcceleration;
class PlayerDeceleration;
class PlayerPoison;

class TimedEffects {
public:
    TimedEffects();
    ~TimedEffects();
    TimedEffects(const TimedEffects &) = delete;
    TimedEffects(TimedEffects &&) = delete;
    TimedEffects &operator=(const TimedEffects &) = delete;
    TimedEffects &operator=(TimedEffects &&) = delete;

    PlayerBlindness &blindness();
    const PlayerBlindness &blindness() const;
    PlayerConfusion &confusion();
    const PlayerConfusion &confusion() const;
    PlayerCut &cut();
    const PlayerCut &cut() const;
    PlayerFear &fear();
    const PlayerFear &fear() const;
    PlayerHallucination &hallucination();
    const PlayerHallucination &hallucination() const;
    PlayerParalysis &paralysis();
    const PlayerParalysis &paralysis() const;
    PlayerStun &stun();
    const PlayerStun &stun() const;
    PlayerAcceleration &acceleration();
    const PlayerAcceleration &acceleration() const;
    PlayerDeceleration &deceleration();
    const PlayerDeceleration &deceleration() const;
    PlayerPoison &poison();
    const PlayerPoison &poison() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
