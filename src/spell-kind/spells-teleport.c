﻿/*!
 * todo 少し長いかも
 * @brief テレポート魔法全般
 * @date 2020/06/04
 * @author Hourier
 */

#include "spell-kind/spells-teleport.h"
#include "art-definition/art-bow-types.h"
#include "cmd-io/cmd-save.h"
#include "core/asking-player.h"
#include "dungeon/dungeon.h"
#include "dungeon/quest.h"
#include "effect/effect-characteristics.h"
#include "game-option/birth-options.h"
#include "game-option/play-record-options.h"
#include "game-option/special-options.h"
#include "io/targeting.h"
#include "io/write-diary.h"
#include "main/sound-definitions-table.h"
#include "main/sound-of-music.h"
#include "core/speed-table.h"
#include "monster-race/race-flags-ability2.h"
#include "monster-race/race-flags-resistance.h"
#include "monster-race/race-flags1.h"
#include "monster-race/race-flags7.h"
#include "monster/monster-describer.h"
#include "monster/monster-description-types.h"
#include "monster/monster-info.h"
#include "monster-floor/monster-move.h"
#include "monster-floor/monster-remover.h"
#include "monster/monster-status.h"
#include "monster/monster-update.h"
#include "monster/smart-learn-types.h"
#include "object-enchant/tr-types.h"
#include "object/object-flags.h"
#include "object/object-hook.h"
#include "player/avatar.h"
#include "player/player-move.h"
#include "spell-kind/spells-launcher.h"
#include "spell/spell-types.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"
#include "world/world.h"

/*!
 * @brief モンスターとの位置交換処理 / Switch position with a monster.
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param dir 方向(5ならばグローバル変数 target_col/target_row の座標を目標にする)
 * @return 作用が実際にあった場合TRUEを返す
 */
bool teleport_swap(player_type *caster_ptr, DIRECTION dir)
{
    POSITION tx, ty;
    if ((dir == 5) && target_okay(caster_ptr)) {
        tx = target_col;
        ty = target_row;
    } else {
        tx = caster_ptr->x + ddx[dir];
        ty = caster_ptr->y + ddy[dir];
    }

    if (caster_ptr->anti_tele) {
        msg_print(_("不思議な力がテレポートを防いだ！", "A mysterious force prevents you from teleporting!"));
        return FALSE;
    }

    grid_type *g_ptr;
    g_ptr = &caster_ptr->current_floor_ptr->grid_array[ty][tx];
    if (!g_ptr->m_idx || (g_ptr->m_idx == caster_ptr->riding)) {
        msg_print(_("それとは場所を交換できません。", "You can't trade places with that!"));
        return FALSE;
    }

    if ((g_ptr->info & CAVE_ICKY) || (distance(ty, tx, caster_ptr->y, caster_ptr->x) > caster_ptr->lev * 3 / 2 + 10)) {
        msg_print(_("失敗した。", "Failed to swap."));
        return FALSE;
    }

    monster_type *m_ptr;
    monster_race *r_ptr;
    m_ptr = &caster_ptr->current_floor_ptr->m_list[g_ptr->m_idx];
    r_ptr = &r_info[m_ptr->r_idx];

    (void)set_monster_csleep(caster_ptr, g_ptr->m_idx, 0);

    if (r_ptr->flagsr & RFR_RES_TELE) {
        msg_print(_("テレポートを邪魔された！", "Your teleportation is blocked!"));
        if (is_original_ap_and_seen(caster_ptr, m_ptr))
            r_ptr->r_flagsr |= RFR_RES_TELE;
        return FALSE;
    }

    sound(SOUND_TELEPORT);
    (void)move_player_effect(caster_ptr, ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
    return TRUE;
}

/*!
 * @brief モンスター用テレポート処理
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param dir 方向(5ならばグローバル変数 target_col/target_row の座標を目標にする)
 * @param distance 移動距離
 * @return 作用が実際にあった場合TRUEを返す
 */
bool teleport_monster(player_type *caster_ptr, DIRECTION dir, int distance)
{
    BIT_FLAGS flg = PROJECT_BEAM | PROJECT_KILL;
    return (project_hook(caster_ptr, GF_AWAY_ALL, dir, distance, flg));
}

/*!
 * @brief モンスターのテレポートアウェイ処理 /
 * Teleport a monster, normally up to "dis" grids away.
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param m_idx モンスターID
 * @param dis テレポート距離
 * @param mode オプション
 * @return テレポートが実際に行われたらtrue
 * @details
 * Attempt to move the monster at least "dis/2" grids away.
 * But allow variation to prevent infinite loops.
 */
bool teleport_away(player_type *caster_ptr, MONSTER_IDX m_idx, POSITION dis, teleport_flags mode)
{
    monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[m_idx];
    if (!monster_is_valid(m_ptr))
        return FALSE;

    if ((mode & TELEPORT_DEC_VALOUR) && (((caster_ptr->chp * 10) / caster_ptr->mhp) > 5) && (4 + randint1(5) < ((caster_ptr->chp * 10) / caster_ptr->mhp))) {
        chg_virtue(caster_ptr, V_VALOUR, -1);
    }

    POSITION oy = m_ptr->fy;
    POSITION ox = m_ptr->fx;
    POSITION min = dis / 2;
    int tries = 0;
    POSITION ny = 0, nx = 0;
    bool look = TRUE;
    while (look) {
        tries++;
        if (dis > 200)
            dis = 200;

        for (int i = 0; i < 500; i++) {
            while (TRUE) {
                ny = rand_spread(oy, dis);
                nx = rand_spread(ox, dis);
                POSITION d = distance(oy, ox, ny, nx);
                if ((d >= min) && (d <= dis))
                    break;
            }

            if (!in_bounds(caster_ptr->current_floor_ptr, ny, nx))
                continue;
            if (!cave_monster_teleportable_bold(caster_ptr, m_idx, ny, nx, mode))
                continue;
            if (!(caster_ptr->current_floor_ptr->inside_quest || caster_ptr->current_floor_ptr->inside_arena))
                if (caster_ptr->current_floor_ptr->grid_array[ny][nx].info & CAVE_ICKY)
                    continue;

            look = FALSE;
            break;
        }

        dis = dis * 2;
        min = min / 2;
        const int MAX_TELEPORT_TRIES = 100;
        if (tries > MAX_TELEPORT_TRIES)
            return FALSE;
    }

    sound(SOUND_TPOTHER);
    caster_ptr->current_floor_ptr->grid_array[oy][ox].m_idx = 0;
    caster_ptr->current_floor_ptr->grid_array[ny][nx].m_idx = m_idx;

    m_ptr->fy = ny;
    m_ptr->fx = nx;

    reset_target(m_ptr);
    update_monster(caster_ptr, m_idx, TRUE);
    lite_spot(caster_ptr, oy, ox);
    lite_spot(caster_ptr, ny, nx);

    if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
        caster_ptr->update |= (PU_MON_LITE);

    return TRUE;
}

/*!
 * @brief モンスターを指定された座標付近にテレポートする /
 * Teleport monster next to a grid near the given location
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param m_idx モンスターID
 * @param ty 目安Y座標
 * @param tx 目安X座標
 * @param power テレポート成功確率
 * @param mode オプション
 * @return なし
 */
void teleport_monster_to(player_type *caster_ptr, MONSTER_IDX m_idx, POSITION ty, POSITION tx, int power, teleport_flags mode)
{
    monster_type *m_ptr = &caster_ptr->current_floor_ptr->m_list[m_idx];
    if (!m_ptr->r_idx)
        return;
    if (randint1(100) > power)
        return;

    POSITION ny = m_ptr->fy;
    POSITION nx = m_ptr->fx;
    POSITION oy = m_ptr->fy;
    POSITION ox = m_ptr->fx;

    POSITION dis = 2;
    int min = dis / 2;
    int attempts = 500;
    bool look = TRUE;
    while (look && --attempts) {
        if (dis > 200)
            dis = 200;

        for (int i = 0; i < 500; i++) {
            while (TRUE) {
                ny = rand_spread(ty, dis);
                nx = rand_spread(tx, dis);
                int d = distance(ty, tx, ny, nx);
                if ((d >= min) && (d <= dis))
                    break;
            }

            if (!in_bounds(caster_ptr->current_floor_ptr, ny, nx))
                continue;
            if (!cave_monster_teleportable_bold(caster_ptr, m_idx, ny, nx, mode))
                continue;

            look = FALSE;
            break;
        }

        dis = dis * 2;
        min = min / 2;
    }

    if (attempts < 1)
        return;

    sound(SOUND_TPOTHER);
    caster_ptr->current_floor_ptr->grid_array[oy][ox].m_idx = 0;
    caster_ptr->current_floor_ptr->grid_array[ny][nx].m_idx = m_idx;

    m_ptr->fy = ny;
    m_ptr->fx = nx;

    update_monster(caster_ptr, m_idx, TRUE);
    lite_spot(caster_ptr, oy, ox);
    lite_spot(caster_ptr, ny, nx);

    if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
        caster_ptr->update |= (PU_MON_LITE);
}

/*!
 * @brief プレイヤーのテレポート先選定と移動処理 /
 * Teleport the player to a location up to "dis" grids away.
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param dis 基本移動距離
 * @param is_quantum_effect 量子的効果 (反テレポ無効)によるテレポートアウェイならばTRUE
 * @param mode オプション
 * @return 実際にテレポート処理が行われたらtrue
 * @details
 * <pre>
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 *
 * There was a nasty tendency for a long time; which was causing the
 * player to "bounce" between two or three different spots because
 * these are the only spots that are "far enough" way to satisfy the
 * algorithm.
 *
 * But this tendency is now removed; in the new algorithm, a list of
 * candidates is selected first, which includes at least 50% of all
 * floor grids within the distance, and any single grid in this list
 * of candidates has equal possibility to be choosen as a destination.
 * </pre>
 */
bool teleport_player_aux(player_type *creature_ptr, POSITION dis, bool is_quantum_effect, teleport_flags mode)
{
    if (creature_ptr->wild_mode)
        return FALSE;
    if (!is_quantum_effect && creature_ptr->anti_tele && !(mode & TELEPORT_NONMAGICAL)) {
        msg_print(_("不思議な力がテレポートを防いだ！", "A mysterious force prevents you from teleporting!"));
        return FALSE;
    }

    int candidates_at[MAX_TELEPORT_DISTANCE + 1];
    for (int i = 0; i <= MAX_TELEPORT_DISTANCE; i++)
        candidates_at[i] = 0;

    if (dis > MAX_TELEPORT_DISTANCE)
        dis = MAX_TELEPORT_DISTANCE;

    int left = MAX(1, creature_ptr->x - dis);
    int right = MIN(creature_ptr->current_floor_ptr->width - 2, creature_ptr->x + dis);
    int top = MAX(1, creature_ptr->y - dis);
    int bottom = MIN(creature_ptr->current_floor_ptr->height - 2, creature_ptr->y + dis);
    int total_candidates = 0;
    for (POSITION y = top; y <= bottom; y++) {
        for (POSITION x = left; x <= right; x++) {
            if (!cave_player_teleportable_bold(creature_ptr, y, x, mode))
                continue;

            int d = distance(creature_ptr->y, creature_ptr->x, y, x);
            if (d > dis)
                continue;

            total_candidates++;
            candidates_at[d]++;
        }
    }

    if (0 == total_candidates)
        return FALSE;

    int cur_candidates;
    int min = dis;
    for (cur_candidates = 0; min >= 0; min--) {
        cur_candidates += candidates_at[min];
        if (cur_candidates && (cur_candidates >= total_candidates / 2))
            break;
    }

    int pick = randint1(cur_candidates);

    /* Search again the choosen location */
    POSITION yy, xx = 0;
    for (yy = top; yy <= bottom; yy++) {
        for (xx = left; xx <= right; xx++) {
            if (!cave_player_teleportable_bold(creature_ptr, yy, xx, mode))
                continue;

            int d = distance(creature_ptr->y, creature_ptr->x, yy, xx);
            if (d > dis)
                continue;
            if (d < min)
                continue;

            pick--;
            if (!pick)
                break;
        }

        if (!pick)
            break;
    }

    if (player_bold(creature_ptr, yy, xx))
        return FALSE;

    sound(SOUND_TELEPORT);
#ifdef JP
    if (IS_ECHIZEN(creature_ptr))
        msg_format("『こっちだぁ、%s』", creature_ptr->name);
#endif
    (void)move_player_effect(creature_ptr, yy, xx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
    return TRUE;
}

/*!
 * @brief プレイヤーのテレポート処理メインルーチン
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param dis 基本移動距離
 * @param mode オプション
 * @return なし
 */
void teleport_player(player_type *creature_ptr, POSITION dis, BIT_FLAGS mode)
{
    if (!teleport_player_aux(creature_ptr, dis, FALSE, mode))
        return;

    /* Monsters with teleport ability may follow the player */
    POSITION oy = creature_ptr->y;
    POSITION ox = creature_ptr->x;
    for (POSITION xx = -1; xx < 2; xx++) {
        for (POSITION yy = -1; yy < 2; yy++) {
            MONSTER_IDX tmp_m_idx = creature_ptr->current_floor_ptr->grid_array[oy + yy][ox + xx].m_idx;
            if (tmp_m_idx && (creature_ptr->riding != tmp_m_idx)) {
                continue;
            }

            monster_type *m_ptr = &creature_ptr->current_floor_ptr->m_list[tmp_m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            bool is_resistible = (r_ptr->a_ability_flags2 & RF6_TPORT) != 0;
            is_resistible &= (r_ptr->flagsr & RFR_RES_TELE) == 0;
            is_resistible &= monster_csleep_remaining(m_ptr) == 0;
            if (is_resistible) {
                teleport_monster_to(creature_ptr, tmp_m_idx, creature_ptr->y, creature_ptr->x, r_ptr->level, TELEPORT_SPONTANEOUS);
            }
        }
    }
}

/*!
 * @brief プレイヤーのテレポートアウェイ処理 /
 * @param m_idx アウェイを試みたモンスターID
 * @param target_ptr プレーヤーへの参照ポインタ
 * @param dis テレポート距離
 * @param is_quantum_effect 量子的効果によるテレポートアウェイならばTRUE
 * @return なし
 */
void teleport_player_away(MONSTER_IDX m_idx, player_type *target_ptr, POSITION dis, bool is_quantum_effect)
{
    if (!teleport_player_aux(target_ptr, dis, TELEPORT_PASSIVE, is_quantum_effect))
        return;

    /* Monsters with teleport ability may follow the player */
    POSITION oy = target_ptr->y;
    POSITION ox = target_ptr->x;
    for (POSITION xx = -1; xx < 2; xx++) {
        for (POSITION yy = -1; yy < 2; yy++) {
            MONSTER_IDX tmp_m_idx = target_ptr->current_floor_ptr->grid_array[oy + yy][ox + xx].m_idx;
            bool is_teleportable = tmp_m_idx > 0;
            is_teleportable &= target_ptr->riding != tmp_m_idx;
            is_teleportable &= m_idx != tmp_m_idx;
            if (!is_teleportable) {
                continue;
            }

            monster_type *m_ptr = &target_ptr->current_floor_ptr->m_list[tmp_m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];

            bool is_resistible = (r_ptr->a_ability_flags2 & RF6_TPORT) != 0;
            is_resistible &= (r_ptr->flagsr & RFR_RES_TELE) == 0;
            is_resistible &= monster_csleep_remaining(m_ptr) == 0;
            if (is_resistible) {
                teleport_monster_to(target_ptr, tmp_m_idx, target_ptr->y, target_ptr->x, r_ptr->level, TELEPORT_SPONTANEOUS);
            }
        }
    }
}

/*!
 * @brief プレイヤーを指定位置近辺にテレポートさせる
 * Teleport player to a grid near the given location
 * @param creature_ptr プレーヤーへの参照ポインタ
 * @param ny 目標Y座標
 * @param nx 目標X座標
 * @param mode オプションフラグ
 * @return なし
 * @details
 * <pre>
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 * </pre>
 */
void teleport_player_to(player_type *creature_ptr, POSITION ny, POSITION nx, teleport_flags mode)
{
    if (creature_ptr->anti_tele && !(mode & TELEPORT_NONMAGICAL)) {
        msg_print(_("不思議な力がテレポートを防いだ！", "A mysterious force prevents you from teleporting!"));
        return;
    }

    /* Find a usable location */
    POSITION y, x;
    POSITION dis = 0, ctr = 0;
    while (TRUE) {
        while (TRUE) {
            y = (POSITION)rand_spread(ny, dis);
            x = (POSITION)rand_spread(nx, dis);
            if (in_bounds(creature_ptr->current_floor_ptr, y, x))
                break;
        }

        bool is_anywhere = current_world_ptr->wizard;
        is_anywhere &= (mode & TELEPORT_PASSIVE) == 0;
        is_anywhere
            &= (creature_ptr->current_floor_ptr->grid_array[y][x].m_idx > 0) || creature_ptr->current_floor_ptr->grid_array[y][x].m_idx == creature_ptr->riding;
        if (is_anywhere)
            break;

        if (cave_player_teleportable_bold(creature_ptr, y, x, mode))
            break;

        if (++ctr > (4 * dis * dis + 4 * dis + 1)) {
            ctr = 0;
            dis++;
        }
    }

    sound(SOUND_TELEPORT);
    (void)move_player_effect(creature_ptr, y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
}

void teleport_away_followable(player_type *tracer_ptr, MONSTER_IDX m_idx)
{
    monster_type *m_ptr = &tracer_ptr->current_floor_ptr->m_list[m_idx];
    POSITION oldfy = m_ptr->fy;
    POSITION oldfx = m_ptr->fx;
    bool old_ml = m_ptr->ml;
    POSITION old_cdis = m_ptr->cdis;

    teleport_away(tracer_ptr, m_idx, MAX_SIGHT * 2 + 5, TELEPORT_SPONTANEOUS);

    bool is_followable = old_ml;
    is_followable &= old_cdis <= MAX_SIGHT;
    is_followable &= current_world_ptr->timewalk_m_idx == 0;
    is_followable &= !tracer_ptr->phase_out;
    is_followable &= los(tracer_ptr, tracer_ptr->y, tracer_ptr->x, oldfy, oldfx);
    if (!is_followable)
        return;

    bool follow = FALSE;
    if ((tracer_ptr->muta1 & MUT1_VTELEPORT) || (tracer_ptr->pclass == CLASS_IMITATOR))
        follow = TRUE;
    else {
        BIT_FLAGS flgs[TR_FLAG_SIZE];
        object_type *o_ptr;
        INVENTORY_IDX i;

        for (i = INVEN_RARM; i < INVEN_TOTAL; i++) {
            o_ptr = &tracer_ptr->inventory_list[i];
            if (o_ptr->k_idx && !object_is_cursed(o_ptr)) {
                object_flags(o_ptr, flgs);
                if (have_flag(flgs, TR_TELEPORT)) {
                    follow = TRUE;
                    break;
                }
            }
        }
    }

    if (!follow)
        return;
    if (!get_check_strict(tracer_ptr, _("ついていきますか？", "Do you follow it? "), CHECK_OKAY_CANCEL))
        return;

    if (one_in_(3)) {
        teleport_player(tracer_ptr, 200, TELEPORT_PASSIVE);
        msg_print(_("失敗！", "Failed!"));
    } else {
        teleport_player_to(tracer_ptr, m_ptr->fy, m_ptr->fx, TELEPORT_SPONTANEOUS);
    }

    tracer_ptr->energy_need += ENERGY_NEED();
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
 * @brief 次元の扉処理 /
 * Dimension Door
 * @param caster_ptr プレーヤーへの参照ポインタ
 * @param x テレポート先のX座標
 * @param y テレポート先のY座標
 * @return 目標に指定通りテレポートできたならばTRUEを返す
 */
bool exe_dimension_door(player_type *caster_ptr, POSITION x, POSITION y)
{
    PLAYER_LEVEL plev = caster_ptr->lev;

    caster_ptr->energy_need += (s16b)((s32b)(60 - plev) * ENERGY_NEED() / 100L);

    if (!cave_player_teleportable_bold(caster_ptr, y, x, TELEPORT_SPONTANEOUS) || (distance(y, x, caster_ptr->y, caster_ptr->x) > plev / 2 + 10)
        || (!randint0(plev / 10 + 10))) {
        caster_ptr->energy_need += (s16b)((s32b)(60 - plev) * ENERGY_NEED() / 100L);
        teleport_player(caster_ptr, (plev + 2) * 2, TELEPORT_PASSIVE);
        return FALSE;
    }

    teleport_player_to(caster_ptr, y, x, TELEPORT_SPONTANEOUS);
    return TRUE;
}

/*!
 * @brief 次元の扉処理のメインルーチン /
 * @param caster_ptr プレーヤーへの参照ポインタ
 * Dimension Door
 * @return ターンを消費した場合TRUEを返す
 */
bool dimension_door(player_type *caster_ptr)
{
    DEPTH x = 0, y = 0;
    if (!tgt_pt(caster_ptr, &x, &y))
        return FALSE;

    if (exe_dimension_door(caster_ptr, x, y))
        return TRUE;

    msg_print(_("精霊界から物質界に戻る時うまくいかなかった！", "You fail to exit the astral plane correctly!"));
    return TRUE;
}
