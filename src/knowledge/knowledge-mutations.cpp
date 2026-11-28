/*!
 * @brief 突然変異の一覧を出力する
 * @date 2020/04/24
 * @author Hourier
 */

#include "knowledge/knowledge-mutations.h"
#include "io-dump/dump-util.h"
#include "io/mutations-dump.h"
#include "io/temp-file.h"
#include "system/player-type-definition.h"

/*!
 * @brief 突然変異表示コマンドの実装 / List mutations we have...
 */
void do_cmd_knowledge_mutations(PlayerType *player_ptr)
{
    TempFile fff;
    dump_mutations(player_ptr, fff);
    show_temp_file(player_ptr, fff, _("突然変異", "Mutations"));
}
