/*!
 * @file cfg-reader.cpp
 * @brief BGM・効果音の設定ファイル (.cfg) の読み込み
 */

#include "main/cfg-reader.h"
#include "io/tokenizer.h"
#include "locale/character-encoding.h"
#include "term/z-rand.h"
#include "util/angband-files.h"
#include <algorithm>

// 1つの項目に設定可能な最大ファイル数
constexpr auto SAMPLE_MAX = 16;

/*!
 * @brief マップのキーを生成する
 * @details
 * typeを上位16ビット, valを下位16ビットに設定した値をマップのキーとする。
 * @param type the "actions" value of "term_xtra()". see:z-term.h TERM_XTRA_xxxxx
 * @param val the 2nd parameter of "term_xtra()"
 * @return 生成したキーを返す
 */
static cfg_key make_cfg_key(int type, int val)
{
    return (type << 16) | (val & 0xffff);
}

/*!
 * @brief 登録されている中からランダムに選択する
 * @param key1_type the "actions" value of "term_xtra()". see:z-term.h TERM_XTRA_xxxxx
 * @param key2_val the 2nd parameter of "term_xtra()"
 * @return キーに対応する値、複数のファイル名の中からからランダムに返す。登録されていない場合はtl::nulloptを返す。
 */
tl::optional<std::string> CfgData::get_rand(int key1_type, int key2_val) const
{
    const auto it = this->map.find(make_cfg_key(key1_type, key2_val));
    if (it == this->map.end()) {
        return tl::nullopt;
    }

    // 音を鳴らすかどうかは設定やファイルの有無で変わるため、固定シードで再現する Rand_external() の乱数列は消費しない
    static xso::rng32 rng;
    return rand_choice(rng, it->second);
}

bool CfgData::has_key(int key1_type, int key2_val) const
{
    return this->map.contains(make_cfg_key(key1_type, key2_val));
}

void CfgData::insert(int key1_type, int key2_val, cfg_values &&value)
{
    this->map.emplace(make_cfg_key(key1_type, key2_val), std::move(value));
}

/*!
 * @param dir .cfgファイルのディレクトリ
 * @param files .cfgファイル名。複数指定可能で、最初に見つかったファイルから読み取る。
 */
CfgReader::CfgReader(const std::filesystem::path &dir, std::initializer_list<concptr> files)
{
    this->dir = dir;
    auto exists_in_dir = [dir](const char *filename) { return is_regular_file(path_build(dir, filename)); };
    if (auto it = std::find_if(files.begin(), files.end(), exists_in_dir); it != files.end()) {
        this->cfg_path = path_build(dir, *it);
    }
}

/*!
 * @brief データの読み込みを行う。
 * @param sections 読み取るデータの指定
 * @param get_value .cfg から値を取り出す関数
 * @return 読み込んだデータを登録したCfgDataを返す。
 * @details 値は空白で区切った複数のファイル名で、ディレクトリに無いファイルは除く。
 */
CfgData CfgReader::read_sections(std::initializer_list<cfg_section> sections, const CfgValueGetter &get_value) const
{
    CfgData result;
    for (const auto &section : sections) {
        tl::optional<std::string> read_key;
        for (auto index = 0; (read_key = section.key_at(index)) != tl::nullopt; ++index) {
            auto value = get_value(section.section_name, *read_key);
            if (value.empty()) {
                continue;
            }
#ifdef JP
            // .cfg (UTF-8) を内部の文字コードにする
            value = guess_convert_to_system_encoding(std::move(value));
#endif
            cfg_values filenames;
            for (const auto token : tokenize_whitespace(value, SAMPLE_MAX)) {
                if (is_regular_file(path_build(this->dir, token))) {
                    filenames.emplace_back(token);
                }
            }
            if (filenames.empty()) {
                continue;
            }

            result.insert(section.action_type, index, std::move(filenames));
            if (section.has_data) {
                *(section.has_data) = true;
            }
        }
    }

    return result;
}
