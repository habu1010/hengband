#include "mind/mind-monk.h"
#include "action/action-limited.h"
#include "core/player-redraw-types.h"
#include "core/player-update-types.h"
#include "io/input-key-acceptor.h"
#include "mind/stances-table.h"
#include "player-base/player-class.h"
#include "player-info/monk-data-type.h"
#include "player/attack-defense-types.h"
#include "player/special-defense-types.h"
#include "status/action-setter.h"
#include "system/player-type-definition.h"
#include "term/screen-processor.h"
#include "util/int-char-converter.h"
#include "view/display-messages.h"

#include "player/player-action.h"

class MonkStanceActionState : public IPlayerActionState {
public:
    MonkStanceActionState(player_type *player_ptr)
        : player_ptr(player_ptr)
        , pc(player_ptr)
    {
    }

    virtual void release()
    {
        if (pc.get_monk_stance() == MonkStance::NONE) {
            msg_print(_("構えをといた。", "You stop assuming the special stance."));
        } else {
            msg_print(_("構えがとけた。", "You lose your stance."));
            pc.set_monk_stance(MonkStance::NONE);
        }
        this->player_ptr->update |= PU_BONUS;
        this->player_ptr->redraw |= PR_STATE;
    }

    virtual void set()
    {
        auto new_stance = pc.get_monk_stance();
        msg_format(_("%sの構えをとった。", "You assume the %s stance."), monk_stances[enum2i(new_stance) - 1].desc);
        this->stance = new_stance;
        this->player_ptr->update |= PU_BONUS;
        this->player_ptr->redraw |= PR_STATE;
    }

    virtual void update() override
    {
        if (pc.monk_stance_is(stance)) {
            msg_print(_("構え直した。", "You reassume a stance."));
        } else {
            this->set();
        }
    }

    virtual std::tuple<term_color_type, std::string> get_state_desc()
    {
        term_color_type attr;
        if (auto stance = pc.get_monk_stance();
            stance != MonkStance::NONE) {
            switch (stance) {
            case MonkStance::GENBU:
                attr = TERM_GREEN;
                break;
            case MonkStance::BYAKKO:
                attr = TERM_WHITE;
                break;
            case MonkStance::SEIRYU:
                attr = TERM_L_BLUE;
                break;
            case MonkStance::SUZAKU:
                attr = TERM_L_RED;
                break;
            default:
                break;
            }
            return { attr, monk_stances[enum2i(stance) - 1].desc };
        }
        return { TERM_WHITE, "    " };
    }

private:
    player_type *player_ptr;
    PlayerClass pc;
    MonkStance stance = MonkStance::NONE;
};

static void set_stance(player_type *player_ptr, const MonkStance new_stance)
{
    auto old_stance = PlayerClass(player_ptr).get_monk_stance();
    PlayerClass(player_ptr).set_monk_stance(new_stance);
    if (old_stance == MonkStance::NONE) {
        player_ptr->action_p.set(std::make_shared<MonkStanceActionState>(player_ptr));
    } else {
        player_ptr->action_p.update();
    }
#if 0
    set_action(player_ptr, ACTION_MONK_STANCE);
    PlayerClass pc(player_ptr);
    if (pc.monk_stance_is(new_stance)) {
        msg_print(_("構え直した。", "You reassume a stance."));
        return;
    }

    player_ptr->update |= PU_BONUS;
    player_ptr->redraw |= PR_STATE;
    msg_format(_("%sの構えをとった。", "You assume the %s stance."), monk_stances[enum2i(new_stance) - 1].desc);
    pc.set_monk_stance(new_stance);
#endif
}

/*!
 * @brief 修行僧の構え設定処理
 * @return 構えを変化させたらTRUE、構え不能かキャンセルしたらFALSEを返す。
 */
bool choose_monk_stance(player_type *player_ptr)
{
    if (cmd_limit_confused(player_ptr))
        return false;

    screen_save();
    prt(_(" a) 構えをとく", " a) No form"), 2, 20);
    for (auto i = 0U; i < monk_stances.size(); i++) {
        if (player_ptr->lev >= monk_stances[i].min_level) {
            char buf[80];
            sprintf(buf, " %c) %-12s  %s", I2A(i + 1), monk_stances[i].desc, monk_stances[i].info);
            prt(buf, 3 + i, 20);
        }
    }

    prt("", 1, 0);
    prt(_("        どの構えをとりますか？", "        Choose Stance: "), 1, 14);

    auto new_stance = MonkStance::NONE;
    while (true) {
        char choice = inkey();
        if (choice == ESCAPE) {
            screen_load();
            return false;
        }

        if ((choice == 'a') || (choice == 'A')) {
            if (!PlayerClass(player_ptr).monk_stance_is(MonkStance::NONE)) {
                PlayerClass(player_ptr).set_monk_stance(MonkStance::NONE);
                player_ptr->action_p.reset();
            } else
                msg_print(_("もともと構えていない。", "You are not in a special stance."));
            screen_load();
            return true;
        }

        if ((choice == 'b') || (choice == 'B')) {
            new_stance = MonkStance::GENBU;
            break;
        } else if (((choice == 'c') || (choice == 'C')) && (player_ptr->lev > 29)) {
            new_stance = MonkStance::BYAKKO;
            break;
        } else if (((choice == 'd') || (choice == 'D')) && (player_ptr->lev > 34)) {
            new_stance = MonkStance::SEIRYU;
            break;
        } else if (((choice == 'e') || (choice == 'E')) && (player_ptr->lev > 39)) {
            new_stance = MonkStance::SUZAKU;
            break;
        }
    }

    set_stance(player_ptr, new_stance);
    player_ptr->redraw |= PR_STATE;
    screen_load();
    return true;
}
