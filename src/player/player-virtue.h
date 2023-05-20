#pragma once

#include <optional>

class PlayerType;
enum class Virtue : short;

class PlayerVirtue {
public:
    PlayerVirtue(PlayerType *player_ptr);

    bool has(Virtue virtue) const;
    std::optional<short> get(Virtue virtue) const;

private:
    PlayerType *player_ptr;
};
