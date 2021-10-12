#pragma once

#include "term/term-color-types.h"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

struct player_type;

class IPlayerActionState {
public:
    virtual ~IPlayerActionState() = default;
    virtual void set() = 0;
    virtual void release() = 0;
    virtual void update() {}
    virtual std::tuple<term_color_type, std::string> get_state_desc() = 0;
};

class PlayerAction {
public:
    PlayerAction();
    void set(std::shared_ptr<IPlayerActionState> action);
    void update();
    void reset();
    void print_state();
    // void set(player_type *player_ptr, PlayerActionType action);
    // bool is(PlayerActionType action) const;
    // PlayerActionType get() const;

    // void add_observer(std::unique_ptr<IPlayerActionOveserver> &&observer);
    // void remove_observer(std::shared_ptr<IPlayerActionOveserver> observer);

private:
    // std::vector<std::shared_ptr<IPlayerActionOveserver>> observer_list;
    std::shared_ptr<IPlayerActionState> action;
};
