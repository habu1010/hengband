/*!
 * @brief 突然変異の一覧を出力する
 * @date 2020/04/24
 * @author Hourier
 */

#include "knowledge/knowledge-mutations.h"
#include "core/show-file.h"
#include "io/mutations-dump.h"
#include "util/angband-tempfile.h"

/*!
 * @brief 突然変異表示コマンドの実装 / List mutations we have...
 */
void do_cmd_knowledge_mutations(PlayerType *player_ptr)
{
    TempFile tempfile;
    if (!tempfile.open()) {
        return;
    }

    dump_mutations(player_ptr, tempfile.fp());
    tempfile.close();

    show_file(player_ptr, true, tempfile.name(), _("突然変異", "Mutations"), 0, 0);
}
