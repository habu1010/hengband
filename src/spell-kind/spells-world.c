﻿#include "spell-kind/spells-world.h"
#include "cmd-io/cmd-save.h"
#include "core/asking-player.h"
#include "dungeon/dungeon.h"
#include "dungeon/quest.h"
#include "floor/floor-town.h"
#include "floor/floor.h"
#include "floor/geometry.h"
#include "floor/wild.h"
#include "game-option/birth-options.h"
#include "game-option/play-record-options.h"
#include "game-option/special-options.h"
#include "grid/grid.h"
#include "io/input-key-acceptor.h"
#include "io/targeting.h"
#include "io/write-diary.h"
#include "main/sound-definitions-table.h"
#include "main/sound-of-music.h"
#include "market/building-util.h"
#include "monster-race/monster-race.h"
#include "monster-race/race-flags-resistance.h"
#include "monster-race/race-flags1.h"
#include "monster/monster-describer.h"
#include "monster/monster-description-types.h"
#include "monster/monster-info.h"
#include "monster-floor/monster-remover.h"
#include "system/monster-type-definition.h"
#include "term/screen-processor.h"
#include "util/int-char-converter.h"
#include "view/display-messages.h"
#include "world/world.h"

/*!
 * todo 変数名が実態と合っているかどうかは要確認
 * テレポート・レベルが効かないモンスターであるかどうかを判定する
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param idx テレポート・レベル対象のモンスター
 */
bool is_teleport_level_ineffective(player_type *caster_ptr, MONSTER_IDX idx)
{
    floor_type *floor_ptr = caster_ptr->current_floor_ptr;
    bool is_special_floor
        = floor_ptr->inside_arena || caster_ptr->phase_out || (floor_ptr->inside_quest && !random_quest_number(caster_ptr, floor_ptr->dun_level));
    bool is_invalid_floor = idx <= 0;
    is_invalid_floor &= quest_number(caster_ptr, floor_ptr->dun_level) || (floor_ptr->dun_level >= d_info[caster_ptr->dungeon_idx].maxdepth);
    is_invalid_floor &= caster_ptr->current_floor_ptr->dun_level >= 1;
    is_invalid_floor &= ironman_downward;
    return is_special_floor || is_invalid_floor;
}

/*!
 * todo cmd-save.h への依存あり。コールバックで何とかしたい
 * @brief プレイヤー及びモンスターをレベルテレポートさせる /
 * Teleport the player one level up or down (random when legal)
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param m_idx テレポートの対象となるモンスターID(0ならばプレイヤー) / If m_idx <= 0, target is player.
 * @return なし
 */
void teleport_level(player_type *creature_ptr, MONSTER_IDX m_idx)
{
    GAME_TEXT m_name[160];
    bool see_m = TRUE;
    if (m_idx <= 0) {
        strcpy(m_name, _("あなた", "you"));
    } else {
        monster_type *m_ptr = &creature_ptr->current_floor_ptr->m_list[m_idx];
        monster_desc(creature_ptr, m_name, m_ptr, 0);
        see_m = is_seen(creature_ptr, m_ptr);
    }

    if (is_teleport_level_ineffective(creature_ptr, m_idx)) {
        if (see_m)
            msg_print(_("効果がなかった。", "There is no effect."));
        return;
    }

    if ((m_idx <= 0) && creature_ptr->anti_tele) {
        msg_print(_("不思議な力がテレポートを防いだ！", "A mysterious force prevents you from teleporting!"));
        return;
    }

    bool go_up;
    if (randint0(100) < 50)
        go_up = TRUE;
    else
        go_up = FALSE;

    if ((m_idx <= 0) && current_world_ptr->wizard) {
        if (get_check("Force to go up? "))
            go_up = TRUE;
        else if (get_check("Force to go down? "))
            go_up = FALSE;
    }

    if ((ironman_downward && (m_idx <= 0)) || (creature_ptr->current_floor_ptr->dun_level <= d_info[creature_ptr->dungeon_idx].mindepth)) {
#ifdef JP
        if (see_m)
            msg_format("%^sは床を突き破って沈んでいく。", m_name);
#else
        if (see_m)
            msg_format("%^s sink%s through the floor.", m_name, (m_idx <= 0) ? "" : "s");
#endif
        if (m_idx <= 0) {
            if (!creature_ptr->current_floor_ptr->dun_level) {
                creature_ptr->dungeon_idx = ironman_downward ? DUNGEON_ANGBAND : creature_ptr->recall_dungeon;
                creature_ptr->oldpy = creature_ptr->y;
                creature_ptr->oldpx = creature_ptr->x;
            }

            if (record_stair)
                exe_write_diary(creature_ptr, DIARY_TELEPORT_LEVEL, 1, NULL);

            if (autosave_l)
                do_cmd_save_game(creature_ptr, TRUE);

            if (!creature_ptr->current_floor_ptr->dun_level) {
                creature_ptr->current_floor_ptr->dun_level = d_info[creature_ptr->dungeon_idx].mindepth;
                prepare_change_floor_mode(creature_ptr, CFM_RAND_PLACE);
            } else {
                prepare_change_floor_mode(creature_ptr, CFM_SAVE_FLOORS | CFM_DOWN | CFM_RAND_PLACE | CFM_RAND_CONNECT);
            }

            creature_ptr->leaving = TRUE;
        }
    } else if (quest_number(creature_ptr, creature_ptr->current_floor_ptr->dun_level)
        || (creature_ptr->current_floor_ptr->dun_level >= d_info[creature_ptr->dungeon_idx].maxdepth)) {
#ifdef JP
        if (see_m)
            msg_format("%^sは天井を突き破って宙へ浮いていく。", m_name);
#else
        if (see_m)
            msg_format("%^s rise%s up through the ceiling.", m_name, (m_idx <= 0) ? "" : "s");
#endif

        if (m_idx <= 0) {
            if (record_stair)
                exe_write_diary(creature_ptr, DIARY_TELEPORT_LEVEL, -1, NULL);

            if (autosave_l)
                do_cmd_save_game(creature_ptr, TRUE);

            prepare_change_floor_mode(creature_ptr, CFM_SAVE_FLOORS | CFM_UP | CFM_RAND_PLACE | CFM_RAND_CONNECT);

            leave_quest_check(creature_ptr);
            creature_ptr->current_floor_ptr->inside_quest = 0;
            creature_ptr->leaving = TRUE;
        }
    } else if (go_up) {
#ifdef JP
        if (see_m)
            msg_format("%^sは天井を突き破って宙へ浮いていく。", m_name);
#else
        if (see_m)
            msg_format("%^s rise%s up through the ceiling.", m_name, (m_idx <= 0) ? "" : "s");
#endif

        if (m_idx <= 0) {
            if (record_stair)
                exe_write_diary(creature_ptr, DIARY_TELEPORT_LEVEL, -1, NULL);

            if (autosave_l)
                do_cmd_save_game(creature_ptr, TRUE);

            prepare_change_floor_mode(creature_ptr, CFM_SAVE_FLOORS | CFM_UP | CFM_RAND_PLACE | CFM_RAND_CONNECT);
            creature_ptr->leaving = TRUE;
        }
    } else {
#ifdef JP
        if (see_m)
            msg_format("%^sは床を突き破って沈んでいく。", m_name);
#else
        if (see_m)
            msg_format("%^s sink%s through the floor.", m_name, (m_idx <= 0) ? "" : "s");
#endif

        if (m_idx <= 0) {
            if (record_stair)
                exe_write_diary(creature_ptr, DIARY_TELEPORT_LEVEL, 1, NULL);
            if (autosave_l)
                do_cmd_save_game(creature_ptr, TRUE);

            prepare_change_floor_mode(creature_ptr, CFM_SAVE_FLOORS | CFM_DOWN | CFM_RAND_PLACE | CFM_RAND_CONNECT);
            creature_ptr->leaving = TRUE;
        }
    }

    if (m_idx <= 0) {
        sound(SOUND_TPLEVEL);
        return;
    }

    monster_type *m_ptr = &creature_ptr->current_floor_ptr->m_list[m_idx];
    check_quest_completion(creature_ptr, m_ptr);
    if (record_named_pet && is_pet(m_ptr) && m_ptr->nickname) {
        char m2_name[MAX_NLEN];

        monster_desc(creature_ptr, m2_name, m_ptr, MD_INDEF_VISIBLE);
        exe_write_diary(creature_ptr, DIARY_NAMED_PET, RECORD_NAMED_PET_TELE_LEVEL, m2_name);
    }

    delete_monster_idx(creature_ptr, m_idx);
    sound(SOUND_TPLEVEL);
}

bool teleport_level_other(player_type *caster_ptr)
{
    if (!target_set(caster_ptr, TARGET_KILL))
        return FALSE;
    MONSTER_IDX target_m_idx = caster_ptr->current_floor_ptr->grid_array[target_row][target_col].m_idx;
    if (!target_m_idx)
        return TRUE;
    if (!player_has_los_bold(caster_ptr, target_row, target_col))
        return TRUE;
    if (!projectable(caster_ptr, caster_ptr->y, caster_ptr->x, target_row, target_col))
        return TRUE;

    monster_type *m_ptr;
    monster_race *r_ptr;
    m_ptr = &caster_ptr->current_floor_ptr->m_list[target_m_idx];
    r_ptr = &r_info[m_ptr->r_idx];
    GAME_TEXT m_name[MAX_NLEN];
    monster_desc(caster_ptr, m_name, m_ptr, 0);
    msg_format(_("%^sの足を指さした。", "You gesture at %^s's feet."), m_name);

    if ((r_ptr->flagsr & (RFR_EFF_RES_NEXU_MASK | RFR_RES_TELE)) || (r_ptr->flags1 & RF1_QUESTOR)
        || (r_ptr->level + randint1(50) > caster_ptr->lev + randint1(60))) {
        msg_format(_("しかし効果がなかった！", "%^s is unaffected!"), m_name);
    } else {
        teleport_level(caster_ptr, target_m_idx);
    }

    return TRUE;
}

/*!
 * @brief 町間のテレポートを行うメインルーチン
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @return テレポート処理を決定したか否か
 */
bool tele_town(player_type *caster_ptr)
{
    if (caster_ptr->current_floor_ptr->dun_level) {
        msg_print(_("この魔法は地上でしか使えない！", "This spell can only be used on the surface!"));
        return FALSE;
    }

    if (caster_ptr->current_floor_ptr->inside_arena || caster_ptr->phase_out) {
        msg_print(_("この魔法は外でしか使えない！", "This spell can only be used outside!"));
        return FALSE;
    }

    screen_save();
    clear_bldg(4, 10);

    int i;
    int num = 0;
    for (i = 1; i < max_towns; i++) {
        char buf[80];

        if ((i == NO_TOWN) || (i == SECRET_TOWN) || (i == caster_ptr->town_num) || !(caster_ptr->visit & (1L << (i - 1))))
            continue;

        sprintf(buf, "%c) %-20s", I2A(i - 1), town_info[i].name);
        prt(buf, 5 + i, 5);
        num++;
    }

    if (num == 0) {
        msg_print(_("まだ行けるところがない。", "You have not yet visited any town."));
        msg_print(NULL);
        screen_load();
        return FALSE;
    }

    prt(_("どこに行きますか:", "Where do you want to go: "), 0, 0);
    while (TRUE) {
        i = inkey();

        if (i == ESCAPE) {
            screen_load();
            return FALSE;
        }

        else if ((i < 'a') || (i > ('a' + max_towns - 2)))
            continue;
        else if (((i - 'a' + 1) == caster_ptr->town_num) || ((i - 'a' + 1) == NO_TOWN) || ((i - 'a' + 1) == SECRET_TOWN)
            || !(caster_ptr->visit & (1L << (i - 'a'))))
            continue;
        break;
    }

    for (POSITION y = 0; y < current_world_ptr->max_wild_y; y++) {
        for (POSITION x = 0; x < current_world_ptr->max_wild_x; x++) {
            if (wilderness[y][x].town == (i - 'a' + 1)) {
                caster_ptr->wilderness_y = y;
                caster_ptr->wilderness_x = x;
            }
        }
    }

    caster_ptr->leaving = TRUE;
    caster_ptr->leave_bldg = TRUE;
    caster_ptr->teleport_town = TRUE;
    screen_load();
    return TRUE;
}

/*!
 * @brief 現実変容処理
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @return なし
 */
void reserve_alter_reality(player_type *caster_ptr)
{
    if (caster_ptr->current_floor_ptr->inside_arena || ironman_downward) {
        msg_print(_("何も起こらなかった。", "Nothing happens."));
        return;
    }

    if (caster_ptr->alter_reality) {
        caster_ptr->alter_reality = 0;
        msg_print(_("景色が元に戻った...", "The view around you returns to normal..."));
        caster_ptr->redraw |= PR_STATUS;
        return;
    }

    TIME_EFFECT turns = randint0(21) + 15;
    caster_ptr->alter_reality = turns;
    msg_print(_("回りの景色が変わり始めた...", "The view around you begins to change..."));
    caster_ptr->redraw |= PR_STATUS;
}
