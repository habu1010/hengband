/*!
 * @file main-win-cfg-reader.cpp
 * @brief BGM・効果音の設定ファイル (.cfg) の読み込みの Windows の実装
 */

#include "main-win/main-win-define.h"
#include "main/cfg-reader.h"
#define NOMINMAX
#include <windows.h>

/*!
 * @brief データの読み込みを行う。
 * @param sections 読み取るデータの指定
 * @return 読み込んだデータを登録したCfgDataを返す。
 */
CfgData CfgReader::read_sections(std::initializer_list<cfg_section> sections) const
{
    if (!is_regular_file(this->cfg_path)) {
        return {};
    }

    const auto cfg_path_str = this->cfg_path.string();
    return this->read_sections(sections, [&cfg_path_str](const std::string &section, const std::string &key) {
        char buf[MAIN_WIN_MAX_PATH]{};
        const auto length = GetPrivateProfileStringA(section.data(), key.data(), "", buf, MAIN_WIN_MAX_PATH, cfg_path_str.data());
        return std::string(buf, length);
    });
}
