#pragma once

#include "dungeon/quest.h"
#include <iosfwd>
#include <vector>

class PlayerType;
enum class QuestId : short;
void do_cmd_checkquest(PlayerType *player_ptr);
void do_cmd_knowledge_quests_completed(PlayerType *player_ptr, std::ostream &fff, const std::vector<QuestId> &quest_ids);
void do_cmd_knowledge_quests_failed(PlayerType *player_ptr, std::ostream &fff, const std::vector<QuestId> &quest_ids);
void do_cmd_knowledge_quests(PlayerType *player_ptr);
