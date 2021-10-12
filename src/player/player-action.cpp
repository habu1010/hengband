#include "player/player-action.h"
#include "system/player-type-definition.h"
#include "term/screen-processor.h"
#include "window/main-window-row-column.h"

#include <algorithm>

struct NoneActionState : public IPlayerActionState {
    virtual void set() {}
    virtual void release() {}
    virtual void update() override {}
    virtual std::tuple<term_color_type, std::string> get_state_desc()
    {
        return { TERM_WHITE, "    " };
    }
};

PlayerAction::PlayerAction()
    : action(std::make_shared<NoneActionState>())
{
}

void PlayerAction::set(std::shared_ptr<IPlayerActionState> action)
{
    // TODO actionが解除された時の処理
    this->action->release();
    this->action = std::move(action);
    this->action->set();
}

void PlayerAction::update()
{
    this->action->update();
}

void PlayerAction::reset()
{
    this->set(std::make_shared<NoneActionState>());
}

void PlayerAction::print_state()
{
    auto [attr, str] = this->action->get_state_desc();
    c_put_str(attr, format("%5.5s", str.c_str()), ROW_STATE, COL_STATE);
}

#if 0
PlayerActionType PlayerAction::get() const
{
    return this->current_action;
}

bool PlayerAction::is(PlayerActionType action) const
{
    return action == this->get();
}

void PlayerAction::add_observer(std::unique_ptr<IPlayerActionOveserver> &&observer)
{
    this->observer_list.emplace_back(std::move(observer));
}

void PlayerAction::remove_observer(std::shared_ptr<IPlayerActionOveserver> observer)
{
    if (auto it = std::find(observer_list.cbegin(), observer_list.cend(), observer);
        it != observer_list.cend()) {
        observer_list.erase(it);
    }
}
#endif
