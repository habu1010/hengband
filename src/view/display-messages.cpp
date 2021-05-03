﻿#include "view/display-messages.h"
#include "core/window-redrawer.h"
#include "game-option/cheat-options.h"
#include "game-option/input-options.h"
#include "game-option/map-screen-options.h"
#include "game-option/option-flags.h"
#include "io/input-key-acceptor.h"
#include "main/sound-of-music.h"
#include "system/player-type-definition.h"
#include "term/gameterm.h"
#include "term/term-color-types.h"
#include "util/int-char-converter.h"
#include "world/world.h"

#include <deque>
#include <string>

/* Used in msg_print() for "buffering" */
bool msg_flag;

COMMAND_CODE now_message;

namespace {
std::deque<std::string> message_history;
}

/*!
 * @brief 保存中の過去ゲームメッセージの数を返す。 / How many messages are "available"?
 * @return 残っているメッセージの数
 */
s32b message_num(void)
{
    return message_history.size();
}

/*!
 * @brief 過去のゲームメッセージを返す。 / Recall the "text" of a saved message
 * @param age メッセージの世代
 * @return メッセージの文字列ポインタ
 */
concptr message_str(int age)
{
    if ((age < 0) || (age >= message_num()))
        return ("");

    return message_history[age].c_str();
}

static void message_add_aux(std::string str)
{
    std::string splitted;

    if (str.empty())
        return;

    // 80桁を超えるメッセージは80桁ずつ分割する
    if (str.length() > 80) {
        int n;
#ifdef JP
        for (n = 0; n < 80; n++) {
            if (iskanji(str[n])) {
                n++;
            }
        }

        /* 最後の文字が漢字半分 */
        if (n == 81)
            n = 79;
#else
        for (n = 80; n > 60; n--)
            if (str[n] == ' ')
                break;
        if (n == 60)
            n = 80;
#endif
        splitted = str.substr(n);
        str = str.substr(0, n);
    }

    // 直前と同じメッセージの場合、「～ <xNN>」と表示する
    if (!message_history.empty()) {
        const char *t;
        std::string_view last_message = message_history.front();
#ifdef JP
        for (t = last_message.data(); *t && (*t != '<' || (*(t + 1) != 'x')); t++)
            if (iskanji(*t))
                t++;
#else
        for (t = last_message.data(); *t && (*t != '<'); t++)
            ;
#endif
        int j = 1;
        if (*t && t != last_message.data()) {
            if (last_message.length() >= sizeof(" <xN>") - 1) {
                last_message = last_message.substr(0, t - last_message.data() - 1);
                j = atoi(t + 2);
            }
        }

        if (str == last_message && (j < 1000)) {
            str = format("%s <x%d>", str.c_str(), j + 1);
            message_history.pop_front();
            if (!now_message)
                now_message++;
        } else {
            /*流れた行の数を数えておく */
            num_more++;
            now_message++;
        }
    }

    message_history.push_front(std::move(str));

    if (message_history.size() == MESSAGE_MAX)
        message_history.pop_back();

    if (!splitted.empty()) {
        message_add_aux(std::move(splitted));
    }
}

/*!
 * @brief ゲームメッセージをログに追加する。 / Add a new message, with great efficiency
 * @param msg 保存したいメッセージ
 */
void message_add(concptr msg)
{
    message_add_aux(msg);
}

bool is_msg_window_flowed(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        if (angband_term[i] && (window_flag[i] & PW_MESSAGE))
            break;
    }
    if (i < 8) {
        if (num_more < angband_term[i]->hgt)
            return FALSE;

        return (num_more >= 0);
    }
    return (num_more >= 0);
}

/*
 * Hack -- flush
 */
static void msg_flush(player_type *player_ptr, int x)
{
    byte a = TERM_L_BLUE;
    bool show_more = (num_more >= 0);

    if (auto_more && !player_ptr->now_damaged)
        show_more = is_msg_window_flowed();

    if (skip_more)
        show_more = FALSE;

    player_ptr->now_damaged = FALSE;
    if (!player_ptr->playing || show_more) {
        term_putstr(x, 0, -1, a, _("-続く-", "-more-"));
        while (TRUE) {
            int cmd = inkey();
            if (cmd == ESCAPE) {
                /* auto_moreのとき、全て流す */
                num_more = -9999;
                break;
            } else if (cmd == ' ') {
                /* 1画面だけ流す */
                num_more = 0;
                break;
            } else if ((cmd == '\n') || (cmd == '\r')) {
                /* 1行だけ流す */
                num_more--;
                break;
            }

            if (quick_messages)
                break;
            bell();
        }
    }

    term_erase(0, 0, 255);
}

void msg_erase(void) { msg_print(NULL); }

/*!
 * @briefOutput a message to the top line of the screen.
 *
 * Break long messages into multiple pieces (40-72 chars).
 *
 * Allow multiple short messages to "share" the top line.
 *
 * Prompt the user to make sure he has a chance to read them.
 *
 * These messages are memorized for later reference (see above).
 *
 * We could do "term_fresh()" to provide "flicker" if needed.
 *
 * The global "msg_flag" variable can be cleared to tell us to
 * "erase" any "pending" messages still on the screen.
 *
 * Note that we must be very careful about using the
 * "msg_print()" functions without explicitly calling the special
 * "msg_print(NULL)" function, since this may result in the loss
 * of information if the screen is cleared, or if anything is
 * displayed on the top line.
 *
 * Note that "msg_print(NULL)" will clear the top line
 * even if no messages are pending.  This is probably a hack.
 * @todo ここのp_ptrを削除するのは破滅的に作業が増えるので保留
 */
void msg_print(concptr msg)
{
    static int p = 0;
    char *t;
    char buf[1024];

    if (current_world_ptr->timewalk_m_idx)
        return;

    if (!msg_flag) {
        term_erase(0, 0, 255);
        p = 0;
    }

    int n = (msg ? strlen(msg) : 0);
    if (p && (!msg || ((p + n) > 72))) {
        msg_flush(p_ptr, p);
        msg_flag = FALSE;
        p = 0;
    }

    if (!msg)
        return;
    if (n > 1000)
        return;

    if (!cheat_turn) {
        strcpy(buf, msg);
    } else {
        sprintf(buf, ("T:%d - %s"), (int)current_world_ptr->game_turn, msg);
    }

    n = strlen(buf);
    if (current_world_ptr->character_generated)
        message_add(buf);

    t = buf;
    while (n > 72) {
        int check, split = 72;
#ifdef JP
        bool k_flag = FALSE;
        int wordlen = 0;
        for (check = 0; check < 72; check++) {
            if (k_flag) {
                k_flag = FALSE;
                continue;
            }

            if (iskanji(t[check])) {
                k_flag = TRUE;
                split = check;
            } else if (t[check] == ' ') {
                split = check;
                wordlen = 0;
            } else {
                wordlen++;
                if (wordlen > 20)
                    split = check;
            }
        }

#else
        for (check = 40; check < 72; check++) {
            if (t[check] == ' ')
                split = check;
        }
#endif

        char oops = t[split];
        t[split] = '\0';
        term_putstr(0, 0, split, TERM_WHITE, t);
        msg_flush(p_ptr, split + 1);
        t[split] = oops;
        t[--split] = ' ';
        t += split;
        n -= split;
    }

    term_putstr(p, 0, n, TERM_WHITE, t);
    p_ptr->window_flags |= (PW_MESSAGE);
    window_stuff(p_ptr);

    msg_flag = TRUE;
#ifdef JP
    p += n;
#else
    p += n + 1;
#endif

    if (fresh_message)
        term_fresh_force();
}

/*
 * Display a formatted message, using "vstrnfmt()" and "msg_print()".
 */
void msg_format(concptr fmt, ...)
{
    va_list vp;
    char buf[1024];
    va_start(vp, fmt);
    (void)vstrnfmt(buf, 1024, fmt, vp);
    va_end(vp);
    msg_print(buf);
}
