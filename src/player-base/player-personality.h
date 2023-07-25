#pragma once

enum class PlayerPersonalityType;
class PlayerType;
struct player_personality;
class PlayerPersonality {
public:
    PlayerPersonality(PlayerType *player_ptr);
    virtual ~PlayerPersonality() = default;

    bool equals(PlayerPersonalityType ppersonality) const;
    const player_personality *get_info() const;

private:
    PlayerType *player_ptr;
};
