/*!
 * @file unix-cfg-reader.cpp
 * @brief BGM・効果音の設定ファイル (.cfg) の読み込みの Unix 系の実装
 * @details iniファイルの読み込みにSimpleIni(MITライセンス)を使用している
 */

#include "main/cfg-reader.h"
#include <simpleini/SimpleIni.h>

/*!
 * @brief データの読み込みを行う。
 * @param sections 読み取るデータの指定
 * @return 読み込んだデータを登録したCfgDataを返す。
 */
CfgData CfgReader::read_sections(std::initializer_list<cfg_section> sections) const
{
    CSimpleIniA ini;
    if (ini.LoadFile(this->cfg_path.c_str()) < 0) {
        return {};
    }

    return this->read_sections(sections, [&ini](const std::string &section, const std::string &key) {
        return std::string(ini.GetValue(section.data(), key.data(), ""));
    });
}
