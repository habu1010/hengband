/*!
 * @brief 次のレベルに必要な経験値のテスト
 *
 * player/player-status-table.h の calc_next_level_exp() と calc_next_level_exp_x100() が、
 * 表 (アンドロイドは強化値の表) の今のレベルの値に経験値の補正を掛けた値を返すことを確かめる。
 */

#include "player/player-status-table.h"
#include <doctest/doctest.h>

TEST_CASE("calc_next_level_exp returns the experience for the next level")
{
    CHECK_EQ(calc_next_level_exp(1, 100, false), 10);
    CHECK_EQ(calc_next_level_exp(1, 100, true), 20);
    CHECK_EQ(calc_next_level_exp(2, 150, false), 37);
    CHECK_EQ(calc_next_level_exp(2, 150, true), 75);
    CHECK_EQ(calc_next_level_exp(40, 100, false), 1250000);
    CHECK_EQ(calc_next_level_exp(40, 100, true), 1900000);
    CHECK_EQ(calc_next_level_exp(PY_MAX_LEVEL - 1, 100, false), 4500000);
}

TEST_CASE("calc_next_level_exp_x100 returns the value before dividing by 100")
{
    CHECK_EQ(calc_next_level_exp_x100(2, 150, false), 3750);
    CHECK_EQ(calc_next_level_exp_x100(2, 150, true), 7500);
    CHECK_EQ(calc_next_level_exp_x100(10, 125, false), 62500);
}
